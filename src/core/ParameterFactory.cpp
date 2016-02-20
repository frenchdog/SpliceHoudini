// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#include "ParameterFactory.h"
#include "MultiParams.h"

namespace OpenSpliceHoudini
{

ParameterFactory::CreateParameterFuncMap ParameterFactory::m_parameterFuncMap;

void ParameterFactory::RegisterParameter(const std::string& paramTypeName, CreateParameterFunc func)
{
    CreateParameterFuncMap::const_iterator it = m_parameterFuncMap.find(paramTypeName);
    if (it == m_parameterFuncMap.end())
    {
        m_parameterFuncMap[paramTypeName] = func;
    }
}

ParameterFactory::CreateParameterFunc ParameterFactory::Get(const std::string& paramTypeName)
{
    CreateParameterFunc parameterFunc = 0;
    CreateParameterFuncMap::const_iterator it = m_parameterFuncMap.find(paramTypeName);
    if (it != m_parameterFuncMap.end())
    {
        parameterFunc = it->second;
    }

    return parameterFunc;
}

void ParameterFactory::RegisterTypes()
{
    ParameterFactory::RegisterParameter(std::string("Float32"), MultiParams::addFloat32Parameter);
    ParameterFactory::RegisterParameter(std::string("Scalar"), MultiParams::addFloat32Parameter);
    ParameterFactory::RegisterParameter(std::string("SInt32"), MultiParams::addSInt32Parameter);
    ParameterFactory::RegisterParameter(std::string("UInt32"), MultiParams::addUInt32Parameter);
    ParameterFactory::RegisterParameter(std::string("Integer"), MultiParams::addIntegerParameter);
    ParameterFactory::RegisterParameter(std::string("Index"), MultiParams::addIndexParameter);
    ParameterFactory::RegisterParameter(std::string("Size"), MultiParams::addSizeParameter);
    ParameterFactory::RegisterParameter(std::string("Count"), MultiParams::addCountParameter);
    ParameterFactory::RegisterParameter(std::string("String"), MultiParams::addStringParameter);
    ParameterFactory::RegisterParameter(std::string("FilePath"), MultiParams::addFilePathParameter);
    ParameterFactory::RegisterParameter(std::string("Vec3"), MultiParams::addVec3Parameter);
}

} // End namespace OpenSpliceHoudini
