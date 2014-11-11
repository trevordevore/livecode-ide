/* Copyright (C) 2003-2013 Runtime Revolution Ltd.
 
 This file is part of LiveCode.
 
 LiveCode is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License v3 as published by the Free
 Software Foundation.
 
 LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

#include <foundation.h>

#include "foundation-private.h"

////////////////////////////////////////////////////////////////////////////////

MCTypeInfoRef kMCNullTypeInfo;
MCTypeInfoRef kMCBooleanTypeInfo;
MCTypeInfoRef kMCNumberTypeInfo;
MCTypeInfoRef kMCStringTypeInfo;
MCTypeInfoRef kMCDataTypeInfo;
MCTypeInfoRef kMCArrayTypeInfo;
MCTypeInfoRef kMCSetTypeInfo;
MCTypeInfoRef kMCListTypeInfo;

////////////////////////////////////////////////////////////////////////////////

static bool __MCTypeInfoIsNamed(MCTypeInfoRef self);

////////////////////////////////////////////////////////////////////////////////

MCValueTypeCode MCTypeInfoGetTypeCode(MCTypeInfoRef self)
{
    MCTypeInfoRef t_actual_typeinfo;
    t_actual_typeinfo = __MCTypeInfoResolve(self);
    return t_actual_typeinfo -> flags & kMCTypeInfoTypeCodeMask;
}

MCNameRef MCTypeInfoGetName(MCTypeInfoRef self)
{
    if (__MCTypeInfoIsNamed(self))
        return self -> named . name;
    return nil;
}

bool MCTypeInfoConforms(MCTypeInfoRef source, MCTypeInfoRef target)
{
    return source == target;
}

bool MCTypeInfoBind(MCNameRef p_name, MCTypeInfoRef p_typeinfo, MCTypeInfoRef& r_typeinfo)
{
    __MCTypeInfo *self;
    if (!__MCValueCreate(kMCValueTypeCodeTypeInfo, self))
        return false;
    
    self -> flags |= 0xff;
    self -> named . name = MCValueRetain(p_name);
    self -> named . typeinfo = __MCTypeInfoResolve(p_typeinfo);
    
    if (MCValueInterAndRelease(self, r_typeinfo))
        return true;
    
    MCValueRelease(self);
    
    return false;
}

bool MCTypeInfoBindAndRelease(MCNameRef p_name, MCTypeInfoRef p_typeinfo, MCTypeInfoRef& r_typeinfo)
{
    if (MCTypeInfoBind(p_name, p_typeinfo, r_typeinfo))
    {
        MCValueRelease(p_typeinfo);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool MCRecordTypeInfoCreate(const MCRecordTypeFieldInfo *p_fields, uindex_t p_field_count, MCTypeInfoRef& r_typeinfo)
{
    __MCTypeInfo *self;
    if (!__MCValueCreate(kMCValueTypeCodeTypeInfo, self))
        return false;
    
    if (!MCMemoryNewArray(p_field_count, self -> record . fields))
    {
        MCMemoryDelete(self);
        return false;
    }
    
    self -> flags |= kMCValueTypeCodeRecord;
    
    for(uindex_t i = 0; i < p_field_count; i++)
    {
        self -> record . fields[i] . name = MCValueRetain(p_fields[i] . name);
        self -> record . fields[i] . type = MCValueRetain(p_fields[i] . type);
    }
    self -> record . field_count = p_field_count;
    
    if (MCValueInterAndRelease(self, r_typeinfo))
        return true;
    
    MCValueRelease(self);
    
    return false;
}

uindex_t MCRecordTypeInfoGetFieldCount(MCTypeInfoRef unresolved_self)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeRecord);
    
    return self -> record . field_count;
}

MCNameRef MCRecordTypeInfoGetFieldName(MCTypeInfoRef unresolved_self, uindex_t p_index)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeRecord);
    MCAssert(self -> record . field_count > p_index);
    
    return self -> record . fields[p_index] . name;
}

MCTypeInfoRef MCRecordTypeInfoGetFieldType(MCTypeInfoRef unresolved_self, uindex_t p_index)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeRecord);
    MCAssert(self -> record . field_count > p_index);
    
    return self -> record . fields[p_index] . type;
}

////////////////////////////////////////////////////////////////////////////////

bool MCHandlerTypeInfoCreate(const MCHandlerTypeFieldInfo *p_fields, uindex_t p_field_count, MCTypeInfoRef p_return_type, MCTypeInfoRef& r_typeinfo)
{
    __MCTypeInfo *self;
    if (!__MCValueCreate(kMCValueTypeCodeTypeInfo, self))
        return false;
    
    if (!MCMemoryNewArray(p_field_count, self -> handler . fields))
    {
        MCMemoryDelete(self);
        return false;
    }
    
    self -> flags |= kMCValueTypeCodeHandler;
    
    for(uindex_t i = 0; i < p_field_count; i++)
    {
        self -> handler . fields[i] . name = MCValueRetain(p_fields[i] . name);
        self -> handler . fields[i] . type = MCValueRetain(p_fields[i] . type);
        self -> handler . fields[i] . mode = p_fields[i] . mode;
    }
    self -> handler . field_count = p_field_count;
    
    self -> handler . return_type = MCValueRetain(p_return_type);
    
    if (MCValueInterAndRelease(self, r_typeinfo))
        return true;
    
    MCValueRelease(self);
    
    return false;
}

MCTypeInfoRef MCHandlerTypeInfoGetReturnType(MCTypeInfoRef unresolved_self)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeHandler);
    
    return self -> handler . return_type;
}

uindex_t MCHandlerTypeInfoGetParameterCount(MCTypeInfoRef unresolved_self)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);

    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeHandler);
    
    return self -> handler . field_count;
}

MCNameRef MCHandlerTypeInfoGetParameterName(MCTypeInfoRef unresolved_self, uindex_t p_index)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeHandler);
    MCAssert(self -> handler . field_count > p_index);
    
    return self -> handler . fields[p_index] . name;
}

MCHandlerTypeFieldMode MCHandlerTypeInfoGetParameterMode(MCTypeInfoRef unresolved_self, uindex_t p_index)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);

    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeHandler);
    MCAssert(self -> handler . field_count > p_index);
    
    return self -> handler . fields[p_index] . mode;
}

MCTypeInfoRef MCHandlerTypeInfoGetParameterType(MCTypeInfoRef unresolved_self, uindex_t p_index)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);

    MCAssert((self -> flags & kMCTypeInfoTypeCodeMask) == kMCValueTypeCodeHandler);
    MCAssert(self -> handler . field_count > p_index);
    
    return self -> handler . fields[p_index] . type;
}

////////////////////////////////////////////////////////////////////////////////

bool MCErrorTypeInfoCreate(MCNameRef p_domain, MCStringRef p_message, MCTypeInfoRef& r_typeinfo)
{
    __MCTypeInfo *self;
    if (!__MCValueCreate(kMCValueTypeCodeTypeInfo, self))
        return false;
    
    self -> error . domain = MCValueRetain(p_domain);
    self -> error . message = MCValueRetain(p_message);
    
    return true;
}

MCNameRef MCErrorTypeInfoGetDomain(MCTypeInfoRef unresolved_self)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    return self -> error . domain;
}

MCStringRef MCErrorTypeInfoGetMessage(MCTypeInfoRef unresolved_self)
{
    MCTypeInfoRef self;
    self = __MCTypeInfoResolve(unresolved_self);
    
    return self -> error . message;
}

////////////////////////////////////////////////////////////////////////////////

static bool __MCTypeInfoCreateBuiltin(MCValueTypeCode p_code, MCTypeInfoRef& r_typeinfo)
{
    __MCTypeInfo *self;
    if (!__MCValueCreate(kMCValueTypeCodeTypeInfo, self))
        return false;
    
    self -> flags |= p_code & 0xff;
    
    if (MCValueInterAndRelease(self, r_typeinfo))
        return true;
    
    MCValueRelease(self);
    
    return false;
}

static MCValueTypeCode __MCTypeInfoGetTypeCode(MCTypeInfoRef self)
{
    return (self -> flags & kMCTypeInfoTypeCodeMask);
}

static bool __MCTypeInfoIsNamed(MCTypeInfoRef self)
{
    return __MCTypeInfoGetTypeCode(self) == 0xff;
}

MCTypeInfoRef __MCTypeInfoResolve(MCTypeInfoRef self)
{
    if (__MCTypeInfoIsNamed(self))
        return self -> named . typeinfo;
    return self;
}

////////////////////////////////////////////////////////////////////////////////

void __MCTypeInfoDestroy(__MCTypeInfo *self)
{
    if (__MCTypeInfoIsNamed(self))
    {
        MCValueRelease(self -> named . name);
        MCValueRelease(self -> named . typeinfo);
    }
    else if (__MCTypeInfoGetTypeCode(self) == kMCValueTypeCodeRecord)
    {
        for(uindex_t i = 0; i < self -> record . field_count; i++)
        {
            MCValueRelease(self -> record . fields[i] . name);
            MCValueRelease(self -> record . fields[i] . type);
        }
        MCMemoryDeleteArray(self -> record . fields);
    }
    else if (__MCTypeInfoGetTypeCode(self) == kMCValueTypeCodeHandler)
    {
        for(uindex_t i = 0; i < self -> handler . field_count; i++)
        {
            MCValueRelease(self -> handler . fields[i] . name);
            MCValueRelease(self -> handler . fields[i] . type);
        }
        MCValueRelease(self -> handler . return_type);
        MCMemoryDeleteArray(self -> handler . fields);
    }
    else if (__MCTypeInfoGetTypeCode(self) == kMCValueTypeCodeError)
    {
        MCValueRelease(self -> error . domain);
        MCValueRelease(self -> error . message);
    }
}

hash_t __MCTypeInfoHash(__MCTypeInfo *self)
{
    hash_t t_hash;
    t_hash = 0;
    
    uint32_t t_code;
    t_code = __MCTypeInfoGetTypeCode(self);
    
    t_hash = MCHashBytesStream(t_hash, &t_code, sizeof(uint32_t));
    if (__MCTypeInfoIsNamed(self))
        t_hash = MCHashBytesStream(t_hash, &self -> named, sizeof(self -> named));
    else if (t_code == kMCValueTypeCodeRecord)
    {
        t_hash = MCHashBytesStream(t_hash, &self -> record . field_count, sizeof(self -> record . field_count));
        t_hash = MCHashBytesStream(t_hash, self -> record . fields, sizeof(MCRecordTypeFieldInfo) * self -> record . field_count);
    }
    else if (t_code == kMCValueTypeCodeHandler)
    {
        t_hash = MCHashBytesStream(t_hash, &self -> handler . field_count, sizeof(self -> handler . field_count));
        t_hash = MCHashBytesStream(t_hash, &self -> handler . return_type, sizeof(self -> handler . return_type));
        t_hash = MCHashBytesStream(t_hash, self -> handler . fields, sizeof(MCRecordTypeFieldInfo) * self -> handler . field_count);
    }
    else if (t_code == kMCValueTypeCodeError)
    {
        t_hash = MCHashBytesStream(t_hash, &self -> error . domain, sizeof(self -> error . domain));
        t_hash = MCHashBytesStream(t_hash, self -> error . message, sizeof(self -> error . message));
    }
    
    return t_hash;
}

bool __MCTypeInfoIsEqualTo(__MCTypeInfo *self, __MCTypeInfo *other_self)
{
    if (__MCTypeInfoGetTypeCode(self) != __MCTypeInfoGetTypeCode(other_self))
        return false;
    
    MCValueTypeCode t_code;
    t_code = __MCTypeInfoGetTypeCode(self);
    if (t_code == 0xff)
        return MCNameIsEqualTo(self -> named . name, other_self -> named . name) &&
                    self -> named . typeinfo == other_self -> named . typeinfo;
    
    if (t_code == kMCValueTypeCodeRecord)
    {
        if (self -> record . field_count != other_self -> record . field_count)
            return false;
        
        for(uindex_t i = 0; i < self -> record . field_count; i++)
            if (!MCNameIsEqualTo(self -> record . fields[i] . name, other_self -> record . fields[i] . name) ||
                self -> record . fields[i] . type != other_self -> record . fields[i] . type)
                return false;
    }
    else if (t_code == kMCValueTypeCodeHandler)
    {
        if (self -> handler . field_count != other_self -> handler . field_count)
            return false;
        if (self -> handler . return_type != other_self -> handler . return_type)
            return false;
        
        for(uindex_t i = 0; i < self -> handler . field_count; i++)
            if (!MCNameIsEqualTo(self -> handler . fields[i] . name, other_self -> handler . fields[i] . name) ||
                self -> handler . fields[i] . type != other_self -> handler . fields[i] . type ||
                self -> handler . fields[i] . mode != other_self -> handler . fields[i] . mode)
                return false;
    }
    else if (t_code == kMCValueTypeCodeError)
    {
        if (self -> error . domain != other_self -> error . domain)
            return false;
        if (self -> error . message != other_self -> error . message)
            return false;
    }
    
    return true;
}

bool __MCTypeInfoCopyDescription(__MCTypeInfo *self, MCStringRef& r_description)
{
    return false;
}

bool __MCTypeInfoInitialize(void)
{
    return
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeNull, kMCNullTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeBoolean, kMCBooleanTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeNumber, kMCNumberTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeString, kMCStringTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeData, kMCDataTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeArray, kMCArrayTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeList, kMCListTypeInfo) &&
        __MCTypeInfoCreateBuiltin(kMCValueTypeCodeSet, kMCSetTypeInfo);
}

void __MCTypeInfoFinalize(void)
{
    MCValueRelease(kMCNullTypeInfo);
    MCValueRelease(kMCBooleanTypeInfo);
    MCValueRelease(kMCNumberTypeInfo);
    MCValueRelease(kMCStringTypeInfo);
    MCValueRelease(kMCDataTypeInfo);
    MCValueRelease(kMCArrayTypeInfo);
    MCValueRelease(kMCListTypeInfo);
    MCValueRelease(kMCSetTypeInfo);
}

////////////////////////////////////////////////////////////////////////////////
