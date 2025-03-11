#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec2
    ////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct Vec2Type;

    template<> struct Vec2Type<bool>            { using Type = glm::bvec2; };
    template<> struct Vec2Type<int>             { using Type = glm::ivec2; };
    template<> struct Vec2Type<unsigned int>    { using Type = glm::uvec2; };
    template<> struct Vec2Type<float>           { using Type = glm::vec2; };
    template<> struct Vec2Type<double>          { using Type = glm::dvec2; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec3
    ////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct Vec3Type;

    template<> struct Vec3Type<bool>            { using Type = glm::bvec3; };
    template<> struct Vec3Type<int>             { using Type = glm::ivec3; };
    template<> struct Vec3Type<unsigned int>    { using Type = glm::uvec3; };
    template<> struct Vec3Type<float>           { using Type = glm::vec3; };
    template<> struct Vec3Type<double>          { using Type = glm::dvec3; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec4
    ////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct Vec4Type;

    template<> struct Vec4Type<bool>            { using Type = glm::bvec4; };
    template<> struct Vec4Type<int>             { using Type = glm::ivec4; };
    template<> struct Vec4Type<unsigned int>    { using Type = glm::uvec4; };
    template<> struct Vec4Type<float>           { using Type = glm::vec4; };
    template<> struct Vec4Type<double>          { using Type = glm::dvec4; };

} 