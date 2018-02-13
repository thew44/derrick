/**
 * @file        derrick.c
 * @author      Mathieu Allory
 * @date        February 2018
 * @brief       Derrick DFS: deep file search and indexing library
 * @ref         https://github.com/thew44/derrick
 *
 * @details     Implementation of Derrick DFS, self-contained in a single C file
 *
 * @license     MIT License
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "derrick.h"

char* derrick_internal_find_line(const char* i_where)
{
    const char* start = i_where;
    char c = *start;
    while (c != 0 && c != '\n' && c != '\r')
    {
        start--;
        c = *(start);
    }
    start++;

    const char* end = i_where;
    c = *end;
    while (c != 0 && c != '\n' && c != '\r')
    {
        end++;
        c = *(end);
    }
    end--;

    size_t length = end - start + 1;
    char* result = malloc(sizeof(char)*(length + 1));
    strncpy(result, start, length);
    result[length] = 0;
    return result;
}

int derrick_internal_CalculateBufferSize(const char *sDir, LARGE_INTEGER* total_size, size_t* o_number_of_entries)
{
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", sDir);

    if((hFind = FindFirstFileA(sPath, (LPWIN32_FIND_DATAA)&fdFile)) == INVALID_HANDLE_VALUE)
    {
        return DERRICK_PATH_NOT_FOUND;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                derrick_internal_CalculateBufferSize(sPath, total_size, o_number_of_entries); //Recursion, I love it!
            }
            else
            {
                HANDLE hFile = CreateFileA(sPath,           // name of the file
                                   GENERIC_READ,           // open for reading
                                   0,                      // do not share
                                   NULL,                   // default security
                                   OPEN_EXISTING,          // create new file only
                                   FILE_ATTRIBUTE_NORMAL,  // normal file
                                   NULL);                  // no attr. template
                LARGE_INTEGER this_size;
                this_size.QuadPart = 0;
                if (GetFileSizeEx(hFile, &this_size) == 0)
                { 
                    return DERRICK_ERROR;
                }

                CloseHandle(hFile);
                total_size->QuadPart += this_size.QuadPart + sizeof(struct Derrick_EntryHeader_s) + strlen(sPath) + 1;
                (*o_number_of_entries)++;
            }
        }
    }
    while(FindNextFileA(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return DERRICK_OK;
}

int derrick_internal_FillBuffer(const char *sDir, size_t* offset, DerrickIndex io_index)
{
    if (sDir == 0 || io_index == 0) return DERRICK_ERROR;

    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", sDir);

    if((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return DERRICK_PATH_NOT_FOUND;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                derrick_internal_FillBuffer(sPath, offset, io_index);
            }
            else
            {
                HANDLE hFile = CreateFileA(sPath,           // name of the write
                                   GENERIC_READ,           // open for writing
                                   0,                      // do not share
                                   NULL,                   // default security
                                   OPEN_EXISTING,          // create new file only
                                   0,  // normal file
                                   0);                  // no attr. template

                LARGE_INTEGER this_size;
                this_size.QuadPart = 0;
                if (GetFileSizeEx(hFile, &this_size) == 0)
                {
                    return DERRICK_ERROR;
                }

                HANDLE hMapFile = CreateFileMapping(
                    hFile,
                    NULL,                    // default security
                    PAGE_READONLY,          // read/write access
                    0,                       // max. object size
                    0,                // buffer size
                    NULL);                 // name of mapping object

                if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
                {
                    return DERRICK_ERROR;
                }

                LPCTSTR pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_READ, // read/write permission
                    0,
                    0,
                    0);

                if (pBuf == NULL)
                {
                    return DERRICK_ERROR;
                }

                struct Derrick_EntryHeader_s* header = (struct Derrick_EntryHeader_s*)((BYTEP*)(io_index->index) + (*offset));
                header->size = this_size.QuadPart;
                strcpy(header->name, sPath);
                header->name[strlen(sPath)] = 0;
                size_t header_offset = sizeof(struct Derrick_EntryHeader_s) + strlen(sPath) + 1;
                CopyMemory((PVOID)((BYTEP*)(io_index->index) + (*offset) + header_offset), pBuf, this_size.QuadPart);
                (*offset) += this_size.QuadPart + header_offset;
                (io_index->number_of_entries)++;


                UnmapViewOfFile(pBuf);
                CloseHandle(hFile);
            }
        }
    }
    while(FindNextFileA(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return DERRICK_OK;
}

char* Entry_File(struct Derrick_EntryHeader_s* i_header)
{
    // Jump to end of current descriptor
    return (char*)((BYTEP*)i_header + sizeof(struct Derrick_EntryHeader_s) + strlen(i_header->name) + 1);
}

struct Derrick_EntryHeader_s* Entry_Next(struct Derrick_EntryHeader_s* i_header)
{
    // Jump to next entry descriptor
    return (struct Derrick_EntryHeader_s*)((BYTEP*)(Entry_File(i_header)) + i_header->size);
}

void derrick_index_search(DerrickIndex i_index, const char* i_searchfor, Derrick_Parameters io_cb)
{
    if (io_cb == 0 || i_index == 0 || i_searchfor == 0) return;

    size_t cur_idx_cnt = 0;
    struct Derrick_EntryHeader_s* cur_idx = i_index->index;

    while (cur_idx_cnt < i_index->number_of_entries)
    {
        // Check for substring
        char* where = 0;
        if(strstr(cur_idx->name, i_searchfor) !=0)
        {
            io_cb->cd_found(io_cb->ctx_found, cur_idx->name, 0);
        }
        else
        if ((where = strstr(Entry_File(cur_idx), i_searchfor)) != 0)
        {
            if (io_cb->cd_found)
            {
                char* line = derrick_internal_find_line(where);
                io_cb->cd_found(io_cb->ctx_found, cur_idx->name, line);
                free(line);
            }
        }
        cur_idx = Entry_Next(cur_idx);
        cur_idx_cnt++;
    }
}

void derrick_index_list(DerrickIndex i_index)
{
    size_t cur_idx_cnt = 0;
    struct Derrick_EntryHeader_s* cur_idx = (struct Derrick_EntryHeader_s*)(i_index->index);
    while (cur_idx_cnt < i_index->number_of_entries)
    {
        printf("%s (%zub)\n", cur_idx->name, cur_idx->size);
        cur_idx = Entry_Next(cur_idx);
        cur_idx_cnt++;
    }
}

int derrick_index_build(DerrickIndex* io_index, const char* i_path)
{
    // Allocate base structure
    (*io_index) = (struct DerrickIndex_s*)malloc(sizeof(struct DerrickIndex_s));
    (*io_index)->number_of_entries = 0;

    LARGE_INTEGER total_size;
    total_size.QuadPart= 0;
    size_t found_entries = 0;
    int rc = derrick_internal_CalculateBufferSize(i_path, &total_size, &found_entries);
    if (rc != DERRICK_OK) return rc;
    (*io_index)->index = malloc(total_size.QuadPart);
    size_t offset = 0;
    derrick_internal_FillBuffer(i_path, &offset, *io_index);

    return DERRICK_OK;
}

int derrick_deep_search(const char* i_searchfor, const char *i_searchin, Derrick_Parameters io_cb)
{
    if (io_cb == 0 || i_searchin == 0 || i_searchfor == 0) return DERRICK_ERROR;

    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", i_searchin);

    if((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return DERRICK_PATH_NOT_FOUND;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", i_searchin, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                derrick_deep_search(i_searchfor, sPath, io_cb);
            }
            else
            {
                // Verify that file shall not be excluded
                if (io_cb->cb_exclude)
                {
                    if(io_cb->cb_exclude(io_cb->ctx_exclude, sPath) == 1)
                    {
                        continue;
                    }
                }

                // Let's check that file
                HANDLE hFile = CreateFileA(sPath,           // name of the write
                                   GENERIC_READ,           // open for writing
                                   0,                      // do not share
                                   NULL,                   // default security
                                   OPEN_EXISTING,          // create new file only
                                   0,  // normal file
                                   0);                  // no attr. template

                LARGE_INTEGER this_size;
                this_size.QuadPart = 0;
                if (GetFileSizeEx(hFile, &this_size) == 0)
                {
                    return DERRICK_ERROR;
                }

                HANDLE hMapFile = CreateFileMapping(
                    hFile,
                    NULL,                    // default security
                    PAGE_READONLY,          // read/write access
                    0,                       // max. object size
                    0,                // buffer size
                    NULL);                 // name of mapping object

                if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
                {
                    return DERRICK_ERROR;
                }

                LPCTSTR pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_READ, // read/write permission
                    0,
                    0,
                    0);

                if (pBuf == NULL)
                {
                    return DERRICK_ERROR;
                }


                // Compare memory
                void* offset = (void*)pBuf;
                for (size_t i = 0; i < (this_size.QuadPart - strlen(i_searchfor)); ++i)
                {
                    if (0 == memcmp(offset, i_searchfor, strlen(i_searchfor)))
                    {
                        if (io_cb->cd_found)
                        {
                            char* line = derrick_internal_find_line((const char*)offset);
                            io_cb->cd_found(io_cb->ctx_found, sPath, line);
                            free(line);
                        }
                    }
                    ++((BYTEP*)offset);
                }

                UnmapViewOfFile(pBuf);
                CloseHandle(hFile);
            }
        }
    }
    while(FindNextFileA(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return DERRICK_OK;
}

int derrick_count_files(const char* i_searchin, Derrick_Parameters io_cb)
{
    if (io_cb == 0 || i_searchin == 0) return DERRICK_ERROR;

    HANDLE hFind = NULL;
    WIN32_FIND_DATAA fdFile;
    char sPath[2048];
    int number_of_files = 0;

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", i_searchin);

    if((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return DERRICK_PATH_NOT_FOUND;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", i_searchin, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                number_of_files += derrick_count_files(sPath, io_cb);
            }
            else
            {
                // Verify that file shall not be excluded
                if (io_cb->cb_exclude)
                {
                    if(io_cb->cb_exclude(io_cb->ctx_exclude, sPath) == 1)
                    {
                        continue;
                    }
                }

                number_of_files++;
            }
        }
    }
    while(FindNextFileA(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return number_of_files;
}

void derrick_init_parameters(Derrick_Parameters io_cb)
{
    io_cb->cb_exclude = 0;
    io_cb->cd_found = 0;
    io_cb->ctx_exclude = 0;
    io_cb->ctx_found = 0;
    io_cb->param_case_sensitive = 1;
}
