#include "ram_disk.h"

#include "file_system/file_system.h"
#include "string/string.h"
#include "memory/memory.h"

#include "../common.h"

RamFile* ram_disk_load_file(EFI_FILE* volume, CHAR16* path)
{
	EFI_FILE* fileHandle = file_system_open_raw(volume, path);

	RamFile* file = memory_allocate_pool(sizeof(RamFile), EFI_MEMORY_TYPE_RAM_DISK);

	file->size = file_system_get_size(fileHandle);
	file->pageAmount = file->size / 0x1000 + 1;
	file->data = memory_allocate_pages(file->pageAmount, EFI_MEMORY_TYPE_RAM_DISK);
	file_system_read(fileHandle, file->size, file->data);

	memset(file->name, 0, 32);
	char16_to_char(path, file->name);

  	file_system_close(fileHandle);

	return file;
}

RamDirectory* ram_disk_load_directory(EFI_FILE* volume, const char* name)
{
	RamDirectory* dir = memory_allocate_pool(sizeof(RamDirectory), EFI_MEMORY_TYPE_RAM_DISK);

	memset(dir->name, 0, 32);
	strcpy(dir->name, name);
	dir->firstFile = 0;
	dir->lastFile = 0;
	dir->firstChild = 0;
	dir->lastChild = 0;
	dir->next = 0;
	dir->prev = 0;

	while (1) 
	{
		EFI_FILE_INFO* fileInfo;
		UINTN fileInfoSize = 0;

		EFI_STATUS status = uefi_call_wrapper(volume->Read, 3, volume, &fileInfoSize, NULL);
        if (status != EFI_BUFFER_TOO_SMALL) 
		{
            break;
		}

		fileInfo = AllocatePool(fileInfoSize);

		status = file_system_read(volume, fileInfoSize, fileInfo);
		if (EFI_ERROR(status)) 
		{
			Print(L"Error reading file info\n");
			FreePool(fileInfo);
			break;
		}

		if (fileInfo->Attribute & EFI_FILE_DIRECTORY) 
		{
			if (StrCmp(fileInfo->FileName, L".") != 0 && StrCmp(fileInfo->FileName, L"..") != 0) 
			{
				EFI_FILE_PROTOCOL* childVolume = file_system_open_raw(volume, fileInfo->FileName);

				char childName[32];
				char16_to_char(fileInfo->FileName, childName);
				RamDirectory* child = ram_disk_load_directory(childVolume, childName);

				if (dir->firstChild == 0)
				{
					child->next = 0;
					child->prev = 0;

					dir->firstChild = child;
					dir->lastChild = child;
				}
				else
				{
					child->next = 0;
					child->prev = dir->lastChild;

					dir->lastChild->next = child;
					dir->lastChild = child;
				}

				file_system_close(childVolume);
			}
		}
		else
		{
			RamFile* file = ram_disk_load_file(volume, fileInfo->FileName);
				
			if (dir->firstFile == 0)
			{
				file->next = 0;
				file->prev = 0;

				dir->firstFile = file;
				dir->lastFile = file;
			}
			else
			{
				file->next = 0;
				file->prev = dir->lastFile;

				dir->lastFile->next = file;
				dir->lastFile = file;
			}	
		}

		FreePool(fileInfo);
	}

	return dir;
}
