/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DataFileSource.h"
#include <cstdio>

namespace
{
    DataFileSource::Source& GetSource()
    {
        static DataFileSource::Source source;
        return source;
    }
}

void DataFileSource::Set(Source source)
{
    GetSource() = std::move(source);
}

bool DataFileSource::Read(std::string const& path, std::vector<uint8>& contents)
{
    if (Source const& source = GetSource(); source && source(path, contents))
        return true;

    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
        return false;

    contents.clear();
    uint8 buffer[65536];
    std::size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
        contents.insert(contents.end(), buffer, buffer + n);

    bool ok = !ferror(f);
    fclose(f);
    return ok;
}
