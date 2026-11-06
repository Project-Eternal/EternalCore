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

#ifndef DATA_FILE_SOURCE_H
#define DATA_FILE_SOURCE_H

#include "Define.h"
#include <functional>
#include <string>
#include <vector>

// Where the data files extracted from the game client (DataDir: dbc/*.dbc, the cinematic cameras' .m2 files) are read from: the files on disk
// by default. A source (set by a module before the data stores are loaded) is asked first with the path the server would open: it returns
// true and fills `contents` with the whole file when it has it, false to let the file be read from disk.
namespace DataFileSource
{
    using Source = std::function<bool(std::string const& path, std::vector<uint8>& contents)>;

    void Set(Source source);

    // the whole file: from the source, else from disk; false when neither has it
    bool Read(std::string const& path, std::vector<uint8>& contents);
}

#endif
