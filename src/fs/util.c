/*
* Copyright (C) 2014 - plutoo
* Copyright (C) 2014 - ichfly
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include <string.h>

#include "util.h"
#include "mem.h"
#include "fs.h"


archive_type* fs_GetArchiveTypeById(u32 id)
{
    u32 i;

    for(i=0; i<ARRAY_SIZE(archive_types); i++) {
        if(archive_types[i].id == id)
            return &archive_types[i];
    }

    return NULL;
}

const char* fs_FlagsToString(u32 flags, char* buf_out)
{
    buf_out[0] = '\0';

    // Convert flags to string representation.
    if(flags & OPEN_READ)
        strcat(buf_out, "R");
    if(flags & OPEN_WRITE)
        strcat(buf_out, "W");
    if(flags & OPEN_CREATE)
        strcat(buf_out, "C");

    return buf_out;
}

const char* fs_AttrToString(u32 attr, char* buf_out)
{
    int has_prev = 0;

    buf_out[0] = '\0';

    // Convert attributes to string representation.
    if(attr & ATTR_ISREADONLY) {
        strcat(buf_out, "IsReadOnly ");
        attr &= ~ATTR_ISREADONLY;
    }
    if(attr & ATTR_ISARCHIVE) {
        strcat(buf_out, "IsArchive ");
        attr &= ~ATTR_ISARCHIVE;
    }
    if(attr & ATTR_ISHIDDEN) {
        strcat(buf_out, "IsHidden ");
        attr &= ~ATTR_ISHIDDEN;
    }
    if(attr & ATTR_ISDIR) {
        strcat(buf_out, "IsDir ");
        attr &= ~ATTR_ISDIR;
    }

    if(attr) {
        strcat(buf_out, "UNKNOWN");
        ERROR("Unknown attr flags %08x encountered.\n", attr);
    }

    return buf_out;
}

const char* fs_PathTypeToString(u32 type)
{

    // Convert type to string representation.
    switch(type) {
    case PATH_INVALID:
        return "INVALID";
    case PATH_EMPTY:
        return "EMPTY";
    case PATH_BINARY:
        return "BINARY";
    case PATH_CHAR:
        return "CHAR";
    case PATH_WCHAR:
        return "WCHAR";
    default:
        return "UNKNOWN";
    }
}

const char* fs_PathToString(u32 type, u32 ptr, u32 size,
                            char* buf_out, size_t size_out)
{
    static const char* hex_digits = "0123456789abcdef";
    u32 i;

    switch(type) {
    case PATH_BINARY:
        if(size >= size_out)
            return "<couldn't fit it>";

        // Dump binary paths in hex.
        for (i = 0; i<size / 2; i++) {
            u8 b = mem_Read8(ptr + i);
            buf_out[2*i] = hex_digits[(b>>4) & 0xF];
            buf_out[2*i+1] = hex_digits[b & 0xF];
        }

        buf_out[2*i] = '\0';
        return buf_out;

    case PATH_CHAR:
        if(size >= size_out)
            return "<couldn't fit it>";

        // Dump ascii paths normally.
        mem_Read(buf_out, ptr, size);
        buf_out[size] = '\0';
        return buf_out;

    case PATH_WCHAR:
        if(size/2 >= size_out)
            return "<couldn't fit it>";

        for(i=0; i<size; i++) {
            u16 b = mem_Read16(ptr + 2*i);

            // Convert unicode -> ascii.
            // If it fails, use question marks.
            if(b >> 8)
                buf_out[i] = '?';
            else
                buf_out[i] = b & 0xFF;
        }

        buf_out[i] = '\0';
        return buf_out;

    default:
        return "";
    }
}

bool fs_IsSafePath(const char* p)
{
    size_t len=strlen(p);

    if(strstr(p, "..") /*|| p[0] == '/'*/) {
        ERROR("Unsafe path: %s\n", p);
        return false;
    }

    return true;
}
