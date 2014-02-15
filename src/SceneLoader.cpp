#include "SceneLoader.h"

#include <fstream>
#include <sstream>
#include <memory>

#include <lua.hpp>
#include <map>

class LuaDeleter {
public:
    void operator()(lua_State *L) {
        lua_close(L);
    }
};

namespace 
{
	typedef std::unique_ptr<lua_State, LuaDeleter> LuaState;
}

#if 0 // For debugging
static void LuaStackDump(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t) {

		case LUA_TSTRING:  /* strings */
			printf("`%s'", lua_tostring(L, i));
			break;

		case LUA_TBOOLEAN:  /* booleans */
			printf(lua_toboolean(L, i) ? "true" : "false");
			break;

		case LUA_TNUMBER:  /* numbers */
			printf("%g", lua_tonumber(L, i));
			break;

		default:  /* other values */
			printf("%s", lua_typename(L, t));
			break;

		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}
#endif

static std::string GetString(lua_State* L, const char* name, const std::string& def = "")
{
    std::string ret = def;

    assert(lua_istable(L, -1));
    lua_getfield(L, -1, name);

    if (!lua_isnil(L, -1))
    {
        ret = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    return ret;
}

// Expects { name={r=number, g=number,b=number, a=number} } on top of stack
static glm::vec4 GetColor4(lua_State* L, const char* name)
{
	assert(lua_istable(L, -1));

	// Push vector table
	lua_getfield(L, -1, name);
	assert(lua_istable(L, -1));

	lua_getfield(L, -1, "r");
	assert(lua_isnumber(L, -1));
	float r = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "g");
	assert(lua_isnumber(L, -1));
	float g = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "b");
	assert(lua_isnumber(L, -1));
	float b = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "a");
	assert(lua_isnumber(L, -1));
	float a = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);

	// Pop vector-table
	lua_pop(L, 1);

	return glm::vec4(r, g, b, a);
}

// Expects { name={x=number, y=number,z=number, w=number} } on top of stack
static glm::vec3 GetVec3(lua_State* L, const char* name)
{
	assert(lua_istable(L, -1));

	// Push vector table
	lua_getfield(L, -1, name);
	assert(lua_istable(L, -1));

	lua_getfield(L, -1, "x");
	assert(lua_isnumber(L, -1));
    float x = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

	lua_getfield(L, -1, "y");
	assert(lua_isnumber(L, -1));
    float y = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

	lua_getfield(L, -1, "z");
	assert(lua_isnumber(L, -1));
    float z = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

	// Pop vector-table
    lua_pop(L, 1);

    return glm::vec3(x, y, z);
}

static float GetFloat(lua_State* L, const char* name)
{
	lua_getfield(L, -1, name);

	assert(lua_isnumber(L, -1));
    float x = (float)lua_tonumber(L, -1);

    lua_pop(L, 1);

    return x;
}

static int GetInt(lua_State* L, const char* name)
{
	lua_getfield(L, -1, name);

	assert(lua_isnumber(L, -1));
    int x = lua_tointeger(L, -1);

    lua_pop(L, 1);

    return x;
}

static int report(lua_State *L, int status) 
{
    const char *msg;
    if (status) {
        msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error with no message)";
        fprintf(stderr, "status=%d, %s\n", status, msg);
        lua_pop(L, 1);
    }
    return status;
}

void luaParseMeshes(LuaState& L, SceneLoader::SceneInfo& outSceneInfo)
{
	assert(lua_istable(L.get(), -1));
	lua_getfield(L.get(), -1, "meshes");

	assert(lua_istable(L.get(), -1));

	size_t numMeshes = lua_objlen(L.get(), -1);

	outSceneInfo.meshes.reserve(numMeshes);

	for (size_t i = 0; i < numMeshes; ++i)
	{
		lua_pushinteger(L.get(), i + 1); // +1 since LUA is 1-indexed
		lua_gettable(L.get(), -2); // Stack: table, mesh-table, mesh

		// Now on top example:
		// { materialIndex=2, position={x=496.308, y=156.829,z=195.465, w=1}, vertices_size=369160, vertices_offset = 0, num_vertices=1857112979, indices_size=43680, indices_offset = 369160, num_indices=10920, radius=78.1125, }

		uint32_t materialIndex = GetInt(L.get(), "material_index");

		SceneLoader::MeshInfo mesh;
		mesh.materialIndex = materialIndex-1; // -1 to zero-index
		mesh.position = GetVec3(L.get(), "position");
		mesh.radius = GetFloat(L.get(), "radius");

		mesh.numVertices = GetInt(L.get(), "num_vertices");
		mesh.vertexDataSize = GetInt(L.get(), "vertices_size");
		mesh.vertexDataOffset = GetInt(L.get(), "vertices_offset");

		mesh.numIndices = GetInt(L.get(), "num_indices");
		mesh.indexDataSize = GetInt(L.get(), "indices_size");
		mesh.indexDataOffset = GetInt(L.get(), "indices_offset");		

		outSceneInfo.meshes.push_back(mesh);

		lua_pop(L.get(), 1); // Stack: table, mesh-table
	}

	lua_pop(L.get(), 1); // pop mesh-table
}

void luaParseMaterials(LuaState& L, SceneLoader::SceneInfo& outSceneInfo)
{
	lua_getfield(L.get(), -1, "materials"); // Stack now: table, materials-table
	assert(lua_istable(L.get(), -1));

	size_t numMaterials = lua_objlen(L.get(), -1);

	outSceneInfo.materials.reserve(numMaterials);

	for (size_t i = 0; i < numMaterials; ++i)
	{
		lua_pushinteger(L.get(), i + 1);
		lua_gettable(L.get(), -2); // Stack now: table, materials-table, material

		// Now on top example:
		// { diffuse={x=0.588, y=0.588,z=0.588, w=1}, diffuse_texture='textures/sponza_thorn_diff.tga', specular={x=0, y=0,z=0, w=1}, ambient={x=0.588, y=0.588,z=0.588, w=1}, height_texture='textures/sponza_thorn_bump.png', emissive={x=0, y=0,z=0, w=1}, shininess=10 },

		SceneLoader::MaterialInfo materialInfo;
		materialInfo.diffuseTexture = GetString(L.get(), "diffuse_texture");
		materialInfo.specularTexture = GetString(L.get(), "specular_texture");
		materialInfo.normalTexture = GetString(L.get(), "normal_texture");
		materialInfo.heightTexture = GetString(L.get(), "height_texture");
		materialInfo.diffuse = GetColor4(L.get(), "diffuse");
		materialInfo.specular = GetColor4(L.get(), "specular");
		materialInfo.ambient = GetColor4(L.get(), "ambient");
		materialInfo.emissive = GetColor4(L.get(), "emissive");
		materialInfo.shininess = GetFloat(L.get(), "shininess");

		outSceneInfo.materials.push_back(materialInfo);

		lua_pop(L.get(), 1); // Stack: table, materials-table
	}

	lua_pop(L.get(), 1); // Stack: table
}

bool SceneLoader::LoadScene(const std::string& file, SceneInfo& outSceneInfo)
{
    LuaState L(luaL_newstate());
	
    int e = report(L.get(), luaL_loadfile(L.get(), file.c_str()));
    if (e != 0)
    {
        fprintf(stderr, "Couldn't load file: %s\n", file.c_str());
        return false;
    }

    int err = lua_pcall(L.get(), 0, 1, 0);
    if (err != 0)
    {
		fprintf(stderr, "Error execution Lua-script in: %s\n", file.c_str());
        return false;
    }

	// The LUA file should return a table
    assert(lua_istable(L.get(), -1)); // 1: table

	luaParseMeshes(L, outSceneInfo);
	luaParseMaterials(L, outSceneInfo);

	outSceneInfo.meshDataFile = GetString(L.get(), "datafile");

    lua_pop(L.get(), 1); // pop table

	return true;
}