// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __PARAMETERFACTORY_H__
#define __PARAMETERFACTORY_H__

#include <string>
#include <map>

class OP_Parameters;

namespace OpenSpliceHoudini
{

class ParameterFactory
{
public:
    typedef void (*CreateParameterFunc)(OP_Parameters* op, const std::string& name);

    static void RegisterParameter(const std::string& paramTypeName, CreateParameterFunc func);
    static void RegisterTypes();

    static CreateParameterFunc Get(const std::string& paramTypeName);

private:
    typedef std::map<std::string, CreateParameterFunc> CreateParameterFuncMap;

    static CreateParameterFuncMap m_parameterFuncMap;
};

} // End namespace OpenSpliceHoudini
#endif // __PARAMETERFACTORY_H__
