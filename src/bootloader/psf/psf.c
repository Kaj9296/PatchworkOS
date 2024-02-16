#include "psf.h"

#include "file_system/file_system.h"
#include "memory/memory.h"

#include <common/common.h>

void pst_font_load(EFI_HANDLE imageHandle, PsfFont* font, CHAR16* path)
{
	EFI_FILE* file = file_system_open(imageHandle, path);

	if (file == NULL)
	{
		Print(L"ERROR: Failed to load font!\n\r");
		
		while (1)
		{
			__asm__("HLT");
		}
	}

	file_system_read(file, sizeof(PsfHeader), &font->header);

	if (font->header.magic != PSF_MAGIC)
	{
		Print(L"ERROR: Invalid font magic found (%d)!\n\r", font->header.magic);
		
		while (1)
		{
			__asm__("HLT");
		}
	}

	uint64_t glyphBufferSize = font->header.charSize * 256;
	if (font->header.mode == 1)
	{
		glyphBufferSize = font->header.charSize * 512;
	}

	void* glyphBuffer = memory_allocate_pages(glyphBufferSize / 0x1000 + 1, EFI_MEMORY_TYPE_SCREEN_FONT);
	file_system_seek(file, sizeof(PsfHeader));
	file_system_read(file, glyphBufferSize, glyphBuffer);

	font->glyphs = glyphBuffer;

	file_system_close(file);

	Print(L"FONT INFO\n\r");
	Print(L"Char Size: %d\n\r", font->header.charSize);
	Print(L"Mode: %d\n\r", font->header.mode);
	Print(L"GlyphBuffer: 0x%x\n\r", glyphBuffer);
	Print(L"FONT INFO END\n\r");
}