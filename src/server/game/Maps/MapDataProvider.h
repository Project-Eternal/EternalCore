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

#ifndef ACORE_MAP_DATA_PROVIDER_H
#define ACORE_MAP_DATA_PROVIDER_H

#include "Define.h"
#include "Optional.h"
#include "GridTerrainData.h"   // (after Optional.h: it uses Optional)
#include <memory>
#include <vector>

class GameObject;
class dtNavMesh;

namespace G3D
{
    class Vector3;
}

namespace VMAP
{
    enum class ModelIgnoreFlags : uint32;
    struct AreaAndLiquidData;
}

// Where the world's terrain and model collision answers come from. By default (no provider) they come from the files extracted from the
// game client: maps/*.map (terrain height, area, liquid) and vmaps/ (line of sight, object hits, height and area/liquid within the
// models). A module may install its own provider (before the maps are created), which then answers every such question instead of the
// files: e.g. a running game client.
//
// Every answer it cannot give must be the server's "no data" value, what the server sees without the files: these are given per method.
// The methods are called from the map update threads at the same time: an implementation must be thread safe. Coordinates are the
// server's (yards; x, y, z), mapId the Map.dbc id.
class MapDataProvider
{
public:
    virtual ~MapDataProvider() = default;

    static void Set(std::shared_ptr<MapDataProvider> provider);
    static MapDataProvider* Get();

    // ---- terrain (in place of maps/MMMXXYY.map) ----

    // whether grid (gx, gy) of the map has terrain (the start-up check of the starting areas; a grid without terrain is not asked)
    virtual bool HasTerrain(uint32 mapId, uint32 gx, uint32 gy) = 0;
    // the terrain's height at x, y: INVALID_HEIGHT when unknown
    virtual float GetTerrainHeight(uint32 mapId, float x, float y) = 0;
    // the lowest point of the terrain's cell at x, y: MIN_HEIGHT when unknown
    virtual float GetTerrainMinHeight(uint32 mapId, float x, float y) = 0;
    // the area (AreaTable.dbc) of the terrain at x, y: 0 when unknown
    virtual uint16 GetTerrainArea(uint32 mapId, float x, float y) = 0;
    // the level of the terrain's liquid at x, y: INVALID_HEIGHT when unknown or none
    virtual float GetTerrainLiquidLevel(uint32 mapId, float x, float y) = 0;
    // the terrain's liquid at a position (GridTerrainData::GetLiquidData): LiquidData() (no water) when unknown or none
    virtual LiquidData GetTerrainLiquidData(uint32 mapId, float x, float y, float z, float collisionHeight, Optional<uint8> reqLiquidType) = 0;

    // ---- models: buildings (WMO) and doodads (M2) ----
    // Asked for one of the two sets the server keeps apart: the map's own models (in place of vmaps/: StaticVMapCollisionData) or the game
    // objects' models (in place of the dynamic tree of GameObjectModels: DynamicVMapCollisionData; doors, transports, destructible
    // buildings as they are now). The server's phase mask of a game object question is not passed on: the provider answers with the
    // objects it has.
    enum class Models { Map, GameObjects };

    // whether nothing of the models blocks the line between the points (ignoreFlags: VMAP::ModelIgnoreFlags::M2 leaves the doodads / the
    // game objects with a doodad model out): true when unknown
    virtual bool IsInLineOfSight(uint32 mapId, Models models, float x1, float y1, float z1, float x2, float y2, float z2,
        VMAP::ModelIgnoreFlags ignoreFlags) = 0;
    // the first hit of the line with the models, moved by modifyDist along the line (rx, ry, rz) and true; false and the end point when
    // nothing is hit or unknown
    virtual bool GetObjectHitPos(uint32 mapId, Models models, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry,
        float& rz, float modifyDist) = 0;
    // the height of the models' floor under x, y, z within maxSearchDist: VMAP_INVALID_HEIGHT_VALUE when none or unknown
    virtual float GetModelHeight(uint32 mapId, Models models, float x, float y, float z, float maxSearchDist) = 0;
    // the building (WMO) under x, y, z: its floor, area information and liquid (VMAP::AreaAndLiquidData) and true; false when none or unknown
    virtual bool GetAreaAndLiquidData(uint32 mapId, Models models, float x, float y, float z, Optional<uint8> reqLiquidType,
        VMAP::AreaAndLiquidData& data) = 0;

    // ---- game objects' models (in place of GameObjectModel, made from vmaps/) ----

    // whether the ray from x, y, z along the unit direction dx, dy, dz hits the model of the game object (where it is now) within maxDist:
    // true and the distance in maxDist; false when it does not or unknown. Asked for game objects that have no GameObjectModel (the
    // transports a unit or object stands on: Map::GetTransportForPos)
    virtual bool IntersectGameObject(GameObject const& go, float x, float y, float z, float dx, float dy, float dz, float& maxDist) = 0;

    // ---- paths (in place of mmaps/) ----

    // the map's navigation mesh (Detour, in the coordinates of mmaps: y, z, x), which the provider fills itself instead of the server
    // loading mmaps/ tiles: nullptr (the default) = the files as before. Asked once per base map.
    virtual std::shared_ptr<dtNavMesh> GetNavMesh(uint32 /*mapId*/) { return nullptr; }
    // a grid of a map whose navigation mesh the provider gives was loaded (where the server would load its mmaps/ tile)
    virtual void NavMeshGridLoaded(uint32 /*mapId*/, uint32 /*gx*/, uint32 /*gy*/) { }
    // a map's update, on the thread that updates it: the only place the provider may change the navigation mesh of the map (the paths are
    // searched in it from the map's update; with several map update threads the maps sharing a mesh (a base map and its instances) are
    // updated at the same time: the provider's mesh then needs one map update thread)
    virtual void UpdateNavMesh(uint32 /*mapId*/, uint32 /*diff*/) { }
    // a path just found in the navigation mesh (its points, the server's coordinates), checked against what the mesh may not know (game
    // objects as they are now): true and the first blocked segment (points[segment] to points[segment + 1]) and where to stop on it;
    // false (the default) when it is clear or unknown
    virtual bool CheckPath(uint32 /*mapId*/, std::vector<G3D::Vector3> const& /*points*/, std::size_t& /*segment*/, G3D::Vector3& /*stop*/)
    {
        return false;
    }
};

#endif
