// Memory_load.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "resource.h"
#include "windows.h"
#define DLL_FILE "F:\\c\\MessageBoxDLL\\Debug\\MessageBoxDLL.dll"
typedef VOID(*MessageProc)(VOID); //定义导出函数类型
#define GET_HEADER_DICTIONARY(module,idx)  &(module)->headers->OptionalHeader.DataDirectory[idx] 
#ifdef _WIN64
#define POINTER_TYPE ULONGLONG
#else
#define POINTER_TYPE DWORD
#endif
static int ProtectionFlags[2][2][2] = {
	{
		// not executable
		{ PAGE_NOACCESS, PAGE_WRITECOPY },
		{ PAGE_READONLY, PAGE_READWRITE },
	},{
		// executable
		{ PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY },
		{ PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE },
	},
};
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION)) //重定位块首部的大小

typedef void *HMEMORYMOUDLE;

typedef BOOL(WINAPI *DllEntryProc)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved); //dll入口函数

typedef struct {
	PIMAGE_NT_HEADERS headers;
	unsigned char *codeBase;
	HMODULE *modules;
	int numModules;
	int initialized;
}MEMORYMOUDLE,*PMEMORYMODULE;
void loadFromFile(void)
{
	HINSTANCE handle = LoadLibrary(DLL_FILE);
	if (handle == NULL)
	{
		printf("LoadLibrary fail %d", GetLastError());
		return;
	}
	MessageProc  mess;
	mess = (MessageProc)GetProcAddress(handle, "DllRegisterServer");
	if (mess == NULL)
	{
		printf("GetProcAddress fail %d", GetLastError());
		return;
	}
	mess();
	FreeLibrary(handle);
}
static void CopySections(const unsigned char*data, PIMAGE_NT_HEADERS nt_headers, PMEMORYMODULE module)
{
	int i, size;
	unsigned char *CodeBase = module->codeBase;
	unsigned char *dest;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);  //获取第一个section头的地址
	for (i = 0; i < module->headers->FileHeader.NumberOfSections;i++, section++)
	{
		if (section->SizeOfRawData == 0)
		{
			size = nt_headers->OptionalHeader.SectionAlignment;
			if (size > 0)
			{
				dest = (unsigned char *)VirtualAlloc(CodeBase + section->VirtualAddress, size, MEM_COMMIT, PAGE_READWRITE);
				section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
				memset(dest, 0, size);
			}
			continue;
		}
		dest = (unsigned char *)VirtualAlloc(CodeBase + section->VirtualAddress, section->SizeOfRawData, MEM_COMMIT, PAGE_READWRITE);
		memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
		section->Misc.PhysicalAddress = (POINTER_TYPE)dest;
	}
}
static void PerformBaseRelocation(PMEMORYMODULE module, SIZE_T delta)
{
	DWORD i;
	unsigned char *codeBase = module->codeBase; //模块加载的基址
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_BASERELOC); //重定位表的地址
	if (directory->Size > 0)
	{
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);//第一个重定位块的地址
		for (; relocation->VirtualAddress > 0;)
		{
			unsigned char *dest = codeBase + relocation->VirtualAddress; //第一个块的起始地址  RVA+VA
			unsigned short*relInfo = (unsigned short *)((unsigned char *)relocation) + IMAGE_SIZEOF_BASE_RELOCATION;
			//(relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2  是每个重定位块中重定位信息的个数
			for (i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++)
			{
				DWORD *patchAddrHL;
#ifdef _WIN64
				ULONGLONG *patchAddr64;
#endif
				int type, offset;
				type = *relInfo >> 12;  //高四位是类型
				offset = *relInfo * 0xfff;//第四位是重定位的偏移
				switch(type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					break;
				case IMAGE_REL_BASED_HIGHLOW:
					patchAddrHL = (DWORD *)dest + offset;
					*patchAddrHL += delta;
					break;
#ifdef _WIN64
				case IMAGE_REL_BASED_DIR64:
					patchAddr64 = (ULONGLONG *)(dest + offset);
					*patchAddr64 += delta;

#endif 
				default:
					printf("unknown relocation :%d", GetLastError());
					break;
				}
			}
		}
		relocation = (PIMAGE_BASE_RELOCATION)(((char*)relocation) + relocation->SizeOfBlock);
 	}

}
static int BuildImportTable(PMEMORYMODULE module)
{
	int result = 1;
	unsigned char *CodeBase = module->codeBase;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT); //导入表的地址
	if (directory->Size > 0)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(CodeBase + directory->VirtualAddress);//第一个dll地址
		for (; !IsBadReadPtr(importDesc, sizeof(PIMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++)
		{
			POINTER_TYPE *thunkRef;  //IMAGE_THUNK_DATA数组
			FARPROC *funcRef;          //IMAGE_THUNK_DATA数组
			HMODULE handle = LoadLibraryA((LPCSTR)(CodeBase + importDesc->Name));
			if (handle ==NULL)
			{
				printf(" LoadLibraryA  fail:%d", GetLastError());
				result = 0;
				break;
			}
			//为模块重新分配内存
			module->modules = (HMODULE*)realloc(module->modules, (module->numModules + 1)*(sizeof(HMODULE)));
			if (module->modules==NULL)
			{
				printf(" realloc  fail:%d", GetLastError());
				result = 0;
				break;
			}
			module->modules[module->numModules++] = handle;\
			if (importDesc->OriginalFirstThunk)
			{
				thunkRef = (POINTER_TYPE*)(CodeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC*)(CodeBase + importDesc->FirstThunk);
			}
			else {
				thunkRef = (POINTER_TYPE*)(CodeBase + importDesc->FirstThunk);
				funcRef = (FARPROC*)(CodeBase + importDesc->FirstThunk);
			}
			for (;*thunkRef;thunkRef++,funcRef++)
			{
				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef))//高位为1 按序号导入
				{
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				}
				else //按名字导入
				{
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(CodeBase + (*thunkRef));
					*funcRef = (FARPROC)GetProcAddress(handle, (LPCSTR)thunkData->Name);
				}
				if (*funcRef == 0)
				{
					result = 0;
					break;
				}
				
			}
			if (!result)
			{
				break;
			}
				
		}
	}
	
	return result;
}
static void FinalizeSections(PMEMORYMODULE module)
{
	int i;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
#ifdef _WIN64
	POINTER_TYPE imageOffset = (module->headers->OptionalHeader.ImageBase & 0xffffffff00000000);
#else
	#define imageOffset 0
#endif 
	for (i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++)
	{
		DWORD protect, oldprotect, size;
		int executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		int readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
		int writeable = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
		if (section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
		{
			//section is not needed
			VirtualFree((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress | imageOffset), section->SizeOfRawData, MEM_DECOMMIT);
			continue;
		}
		protect = ProtectionFlags[executable][readable][writeable];
		if (section->Characteristics &IMAGE_SCN_MEM_NOT_CACHED)
		{
			//节中的数据不会经过缓存
			protect |= PAGE_NOCACHE;
		}
		//确定节的大小
		size = section->SizeOfRawData;//该块在磁盘中的大小 为0说明区块中的数据是未初始化的
		if (size==0)
		{
			if (section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
				size = module->headers->OptionalHeader.SizeOfInitializedData;
			else if (section->Characteristics &IMAGE_SCN_CNT_UNINITIALIZED_DATA)
				size = module->headers->OptionalHeader.SizeOfUninitializedData;
		}
		if (size > 0)
		{
			//改变内存访问标志
			if (VirtualProtect((LPVOID)((POINTER_TYPE)section->Misc.PhysicalAddress|imageOffset),size,protect,&oldprotect)==0)
			{
				printf(" VirtualProtect  fail:%d", GetLastError());
			}
		}

	}
#ifndef _WIN64
#undef imageOffset
#endif
}
void MemoryFreeLibrary(HMEMORYMOUDLE mod)
{
	int i;
	PMEMORYMODULE module = (PMEMORYMODULE)mod;
	if (module != NULL)
	{
		if (module->initialized != 0)
		{
			//detach from process 
			DllEntryProc DllEntry = (DllEntryProc)(module->codeBase + module->headers->OptionalHeader.AddressOfEntryPoint);
			(*DllEntry)((HINSTANCE)module->codeBase, DLL_PROCESS_DETACH, 0);
		}
		if (module->modules != NULL)
		{
			for (i = 0; i < module->numModules; i++)
			{
				if (module->modules[i] != INVALID_HANDLE_VALUE) {
					FreeLibrary(module->modules[i]);
				}
			}
			free(module->modules);
		}
		if (module->codeBase != NULL)
		{
			VirtualFree(module->codeBase, 0, MEM_RELEASE);
		}
		HeapFree(GetProcessHeap(), 0, module);
	}
}
HMEMORYMOUDLE Memoryloadlibrary(const void *data)
{
	PMEMORYMODULE result;
	PIMAGE_DOS_HEADER dos_header;//dos 头
	PIMAGE_NT_HEADERS nt_header; //NT头
	unsigned char *code, *headers;
	SIZE_T locationDelta;  //重定位
	DllEntryProc DllEntry;  //dll 入口
	BOOL successful;
	//第一步检查dos和pe头的合法性
	//获取dos 头
	dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic!=IMAGE_DOS_SIGNATURE)
	{
		printf("获取dos header fail %d", GetLastError());
		return NULL;
	}
	//获取
	nt_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (nt_header->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("获取nt header fail %d", GetLastError());
		return NULL;
	}
	//第二步尝试在预加载的基地址处分配内存
	//首先进行预定
	code = (unsigned char*)VirtualAlloc((LPVOID)(nt_header->OptionalHeader.ImageBase), nt_header->OptionalHeader.SizeOfImage, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (code == NULL)
	{
		//预定失败，重新在其他位置预定空间
		code = (unsigned char*)VirtualAlloc(NULL, nt_header->OptionalHeader.SizeOfImage, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (code == NULL)
		{
			printf("预定空间失败 fail %d", GetLastError());
			return NULL;
		}
	}
	//自定义结构体 MEMORYMODULE空间的分配  ，在进程的默认堆上分配
	result = (PMEMORYMODULE)HeapAlloc(GetProcessHeap(), 0, sizeof(MEMORYMOUDLE));
	result->codeBase = code;  //分配的地址
	result->numModules = 0;
	result->modules = NULL;
	result->initialized = 0;

	VirtualAlloc(code, nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);  //提交预分配的空间
	//第三步 拷贝头和section到code地址处
	headers = code;
	
	memcpy(headers,dos_header,dos_header->e_lfanew + nt_header->OptionalHeader.SizeOfHeaders);
	//dos_header->e_lfanew   DOS头+DOS STUB   nt_header->OptionalHeader.SizeOfHeaders  PE 头
	result->headers = (PIMAGE_NT_HEADERS)&((const unsigned char*)(headers))[dos_header->e_lfanew];

	result->headers->OptionalHeader.ImageBase = (POINTER_TYPE)code;

	CopySections((const unsigned char*)data, nt_header, result);

	//第四步 检查是否需要重定位
	locationDelta = (SIZE_T)(code - nt_header->OptionalHeader.ImageBase);
	if (locationDelta != 0)
	{
		PerformBaseRelocation(result, locationDelta);
	}
    //第五步加载其他依赖的dll
	if (!BuildImportTable(result))
	{
		printf("加载依赖的dll失败 %d", GetLastError());
		goto error;
	}
	//第六步 根据section属性设置内存页的访问属性
	FinalizeSections(result);
	//第七步 获取dll的入口函数指针，并使用DLL_PROCESS_ARRACH调用
	if (result->headers->OptionalHeader.AddressOfEntryPoint != 0)
	{
		DllEntry = (DllEntryProc)(code + result->headers->OptionalHeader.AddressOfEntryPoint);
		if (DllEntry == 0)
		{
			printf("获取dllentry失败  %d", GetLastError());
			goto error;
		}
		successful = (*DllEntry)((HINSTANCE)code, DLL_PROCESS_ATTACH, 0);
		if (!successful)
		{
			printf("cant attach library  %d", GetLastError());
			goto error;
		}
		result->initialized = 1;
	}
	return (HMEMORYMOUDLE)result;
error:
	MemoryFreeLibrary(result);
	return NULL;
}
FARPROC MemoryGetProcAddress(HMEMORYMOUDLE module, const char *name)
{
	unsigned char *codeBase = ((PMEMORYMODULE)module)->codeBase;
	int idx = -1;
	DWORD i, *nameRef;
	WORD *ordinal;
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY((PMEMORYMODULE)module, IMAGE_DIRECTORY_ENTRY_EXPORT);//获取导入表地址
	if (directory->Size == 0)
	{
		printf("NO exports");
		return NULL;
	}
	exports = (PIMAGE_EXPORT_DIRECTORY)(codeBase + directory->VirtualAddress);//only one 
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0)
	{
		printf("NO FUNCTION");
		return NULL;
	}
	nameRef = (DWORD*)(codeBase + exports->AddressOfNames);//定义了名字的函数
	ordinal = (WORD*)(codeBase + exports->AddressOfNameOrdinals);//与名字意义对应，索引值的指针WORD类型
	for (i = 0; i < exports->AddressOfNames; i++, nameRef++, ordinal++)
	{
		if (stricmp(name, (const char*)(codeBase + (*nameRef))) == 0)
		{
			idx = *ordinal;
			break;
		}
	}
	if (idx == -1)
	{
		printf("no this idx");
		return NULL;
	}
	if ((DWORD)idx > exports->NumberOfFunctions)
	{
		printf("NAME NOT MATCH IDX");
	}
	return (FARPROC)(codeBase + (*(DWORD*)(codeBase + exports->AddressOfFunctions + (idx * 4))));
}
void ResourceToFile(char *Name,char *Type)
{
	MessageProc mess;
	//寻找资源
	HRSRC hRes =FindResource(NULL, Name, Type);
	if (hRes == NULL)
	{
		printf("FindResource fail %d", GetLastError());
		return;
	}
	//加载资源
	HGLOBAL hlRes = LoadResource(NULL, hRes);
	if (hlRes == NULL)
	{
		printf("LoadResource fail %d", GetLastError());
		return;
	}
	//锁定资源
	void *Res=LockResource(hlRes);
	if (&Res == NULL) {
		printf("LockResource fail %d", GetLastError());
		return;
	}
	//获取资源大小 
	DWORD dwsize = SizeofResource(NULL, hRes);
	if (dwsize == 0)
		return;
	unsigned char *data = NULL;
	data = (unsigned char *)malloc(dwsize);
	memcpy(data, Res, dwsize);
	GlobalFree(hRes);
	////创建文件
	//WCHAR *TEST = L"test.dat";
	//HANDLE hFile=CreateFile(TEST, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (hFile== INVALID_HANDLE_VALUE)
	//{
	//	printf("CreateFile fail %d", GetLastError());
	//	return;
	//}
	////写入文件
	//DWORD size;
	//if (!WriteFile(hFile, Res, dwsize, &size, NULL))
	//{
	//	printf("WriteFile fail %d", GetLastError());
	//}
	//CloseHandle(hFile);//记得关闭文件句柄，释放资源
	//GlobalFree(hRes);
    //内存加载
	HMEMORYMOUDLE module;
	
	module=Memoryloadlibrary(data);
	if (module == NULL)
	{
		printf("WriteFile fail %d", GetLastError());
		return;
	}
	mess = (MessageProc)MemoryGetProcAddress(module, "DllRegisterServer");
	mess();
	MemoryFreeLibrary(module);

}
VOID loadFromMemory(void)
{
	;
}
int main()
{
	//loadFromFile();
	ResourceToFile(MAKEINTRESOURCE(ID_MESSAGE_DLL), MAKEINTRESOURCE(RC_DLL));//数字转换为字符串指针
    return 0;
}

