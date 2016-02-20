// Copyright (c) 2015, Guillaume Laforge. All rights reserved.

#ifndef __ATTRIBUTETRAITS_H__
#define __ATTRIBUTETRAITS_H__

namespace OpenSpliceHoudini
{

template <typename T>
struct HouToFabAttributeTraits
{
    static const char* getKLTypeName(GA_TypeInfo typeInfo);
    static const char* getFabricMethodName(GA_TypeInfo typeInfo);

    static void setAttribute(GA_ROHandleT<T> handle,
                             GA_TypeInfo typeInfo,
                             GA_Size nbPoints,
                             FabricCore::Client& client,
                             FabricCore::RTVal& polygonMesh,
                             const char* attrName)
    {
        if (!handle.isValid())
            return;

        size_t bufferSize = static_cast<size_t>(nbPoints);
        std::vector<T> buffer(bufferSize);
        handle.getBlock(GA_Offset(), nbPoints, &buffer[0]);
        try
        {
            std::vector<FabricCore::RTVal> args(2);
            args[0] =
                FabricCore::RTVal::ConstructExternalArray(client, getKLTypeName(typeInfo), bufferSize, &buffer[0]);
            args[1] = FabricCore::RTVal::ConstructString(client, attrName);
            polygonMesh.callMethod("", getFabricMethodName(typeInfo), 2, &args[0]);
        }
        catch (FabricCore::Exception e)
        {
            FabricCore::Exception::Throw(
                (std::string("[SOP_FabricDeformer::CreatePolygonMeshRTVal]: ") + e.getDesc_cstr()).c_str());
        }
    }
};

template <>
const char* HouToFabAttributeTraits<int32>::getKLTypeName(GA_TypeInfo typeInfo)
{
    return "UInt32";
}

template <>
const char* HouToFabAttributeTraits<fpreal32>::getKLTypeName(GA_TypeInfo typeInfo)
{
    return "Float32";
}

template <>
const char* HouToFabAttributeTraits<UT_Vector3F>::getKLTypeName(GA_TypeInfo typeInfo)
{
    return "Vec3";
}

template <>
const char* HouToFabAttributeTraits<UT_Vector4F>::getKLTypeName(GA_TypeInfo typeInfo)
{
    const char* result;
    if (typeInfo == GA_TYPE_COLOR)
        result = "Color";
    else
        result = "Vec4";

    return result;
}

template <>
const char* HouToFabAttributeTraits<int32>::getFabricMethodName(GA_TypeInfo typeInfo)
{
    return "setUInt32PointAttribute";
}

template <>
const char* HouToFabAttributeTraits<fpreal32>::getFabricMethodName(GA_TypeInfo typeInfo)
{
    return "setScalarPointAttribute";
}

template <>
const char* HouToFabAttributeTraits<UT_Vector3F>::getFabricMethodName(GA_TypeInfo typeInfo)
{
    return "setVec3PointAttribute";
}

template <>
const char* HouToFabAttributeTraits<UT_Vector4F>::getFabricMethodName(GA_TypeInfo typeInfo)
{
    const char* result;

    if (typeInfo == GA_TYPE_COLOR)
        result = "setColorPointAttribute";
    else
        result = "setVec4PointAttribute";

    return result;
}

} // End namespace OpenSpliceHoudini
#endif // __ATTRIBUTETRAITS_H__