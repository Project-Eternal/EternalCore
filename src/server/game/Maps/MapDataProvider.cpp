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

#include "MapDataProvider.h"

namespace
{
    std::shared_ptr<MapDataProvider>& Installed()
    {
        static std::shared_ptr<MapDataProvider> provider;
        return provider;
    }
}

void MapDataProvider::Set(std::shared_ptr<MapDataProvider> provider)
{
    Installed() = std::move(provider);
}

MapDataProvider* MapDataProvider::Get()
{
    return Installed().get();
}
