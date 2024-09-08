/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     wj32    2009-2016
 *     dmex    2017-2023
 *
 */

#include <ph.h>
#include <apiimport.h>
#include <lsasup.h>

/**
 * Queries variable-sized information for a token. The function allocates a buffer to contain the
 * information.
 *
 * \param TokenHandle A handle to a token. The access required depends on the information class
 * specified.
 * \param TokenInformationClass The information class to retrieve.
 * \param Buffer A variable which receives a pointer to a buffer containing the information. You
 * must free the buffer using PhFree() when you no longer need it.
 */
NTSTATUS PhpQueryTokenVariableSize(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_ PVOID *Buffer
    )
{
    NTSTATUS status;
    PVOID buffer;
    ULONG bufferSize;
    ULONG returnLength;

    returnLength = 0;
    bufferSize = 0x80;
    buffer = PhAllocate(bufferSize);

    status = NtQueryInformationToken(
        TokenHandle,
        TokenInformationClass,
        buffer,
        bufferSize,
        &returnLength
        );

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        PhFree(buffer);
        bufferSize = returnLength;
        buffer = PhAllocate(bufferSize);

        status = NtQueryInformationToken(
            TokenHandle,
            TokenInformationClass,
            buffer,
            bufferSize,
            &returnLength
            );
    }

    if (NT_SUCCESS(status))
    {
        *Buffer = buffer;
    }
    else
    {
        PhFree(buffer);
    }

    return status;
}

/**
 * Queries variable-sized information for a token. The function allocates a buffer to contain the
 * information.
 *
 * \param TokenHandle A handle to a token. The access required depends on the information class
 * specified.
 * \param TokenInformationClass The information class to retrieve.
 * \param Buffer A variable which receives a pointer to a buffer containing the information. You
 * must free the buffer using PhFree() when you no longer need it.
 */
NTSTATUS PhQueryTokenVariableSize(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_ PVOID *Buffer
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenInformationClass,
        Buffer
        );
}

/**
 * Gets a token's user.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param User A variable which receives a pointer to a structure containing the token's user. You
 * must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenUserCopy(
    _In_ HANDLE TokenHandle,
    _Out_ PSID* User
    )
{
    NTSTATUS status;
    ULONG returnLength;
    UCHAR tokenUserBuffer[TOKEN_USER_MAX_SIZE];
    PTOKEN_USER tokenUser = (PTOKEN_USER)tokenUserBuffer;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenUser,
        tokenUser,
        sizeof(tokenUserBuffer),
        &returnLength
        );

    if (NT_SUCCESS(status))
    {
        *User = PhAllocateCopy(tokenUser->User.Sid, PhLengthSid(tokenUser->User.Sid));
    }

    return status;
}

/*
 * Retrieves information about the token user.
 *
 * \param TokenHandle The handle to the token.
 * \param User A pointer to the PH_TOKEN_USER structure that receives the token user information.
 *
 * \return Returns STATUS_SUCCESS if the function succeeds, otherwise an appropriate NTSTATUS error code.
 */
NTSTATUS PhGetTokenUser(
    _In_ HANDLE TokenHandle,
    _Out_ PPH_TOKEN_USER User
    )
{
    ULONG returnLength;

    return NtQueryInformationToken(
        TokenHandle,
        TokenUser,
        User,
        sizeof(PH_TOKEN_USER), // SE_TOKEN_USER
        &returnLength
        );
}

/**
 * Gets a token's owner.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param Owner A variable which receives a pointer to a structure containing the token's owner. You
 * must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenOwnerCopy(
    _In_ HANDLE TokenHandle,
    _Out_ PSID* Owner
    )
{
    NTSTATUS status;
    UCHAR tokenOwnerBuffer[TOKEN_OWNER_MAX_SIZE];
    PTOKEN_OWNER tokenOwner = (PTOKEN_OWNER)tokenOwnerBuffer;
    ULONG returnLength;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenOwner,
        tokenOwner,
        sizeof(tokenOwnerBuffer),
        &returnLength
        );

    if (NT_SUCCESS(status))
    {
        *Owner = PhAllocateCopy(tokenOwner->Owner, PhLengthSid(tokenOwner->Owner));
    }

    return status;
}

NTSTATUS PhGetTokenOwner(
    _In_ HANDLE TokenHandle,
    _Out_ PPH_TOKEN_OWNER Owner
    )
{
    ULONG returnLength;

    return NtQueryInformationToken(
        TokenHandle,
        TokenOwner,
        Owner,
        sizeof(PH_TOKEN_OWNER),
        &returnLength
        );
}

/**
 * Gets a token's primary group.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param PrimaryGroup A variable which receives a pointer to a structure containing the token's
 * primary group. You must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenPrimaryGroup(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_PRIMARY_GROUP *PrimaryGroup
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenPrimaryGroup,
        PrimaryGroup
        );
}

/**
 * Gets a token's discretionary access control list (DACL).
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param DefaultDacl A pointer to an ACL structure assigned by default to any objects created
 * by the user. You must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenDefaultDacl(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_DEFAULT_DACL* DefaultDacl
    )
{
    NTSTATUS status;
    PTOKEN_DEFAULT_DACL defaultDacl;

    status = PhQueryTokenVariableSize(
        TokenHandle,
        TokenDefaultDacl,
        &defaultDacl
        );

    if (NT_SUCCESS(status))
    {
        if (defaultDacl->DefaultDacl)
        {
            *DefaultDacl = defaultDacl;
        }
        else
        {
            status = STATUS_INVALID_SECURITY_DESCR;
            PhFree(defaultDacl);
        }
    }

    return status;
}

/**
 * Gets a token's groups.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param Groups A variable which receives a pointer to a structure containing the token's groups.
 * You must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenGroups(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_GROUPS *Groups
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenGroups,
        Groups
        );
}

/**
 * Get a token's restricted SIDs.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param RestrictedSids A variable which receives a pointer to a structure containing the token's restricted SIDs.
 * You must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenRestrictedSids(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_GROUPS* RestrictedSids
)
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenRestrictedSids,
        RestrictedSids
        );
}

/**
 * Gets a token's privileges.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param Privileges A variable which receives a pointer to a structure containing the token's
 * privileges. You must free the structure using PhFree() when you no longer need it.
 */
NTSTATUS PhGetTokenPrivileges(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_PRIVILEGES *Privileges
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenPrivileges,
        Privileges
        );
}

NTSTATUS PhGetTokenTrustLevel(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_PROCESS_TRUST_LEVEL *TrustLevel
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenProcessTrustLevel,
        TrustLevel
        );
}

NTSTATUS PhGetTokenAppContainerSidCopy(
    _In_ HANDLE TokenHandle,
    _Out_ PSID* AppContainerSid
    )
{
    NTSTATUS status;
    UCHAR tokenAppContainerSidBuffer[TOKEN_APPCONTAINER_SID_MAX_SIZE];
    PTOKEN_APPCONTAINER_INFORMATION tokenAppContainerSid = (PTOKEN_APPCONTAINER_INFORMATION)tokenAppContainerSidBuffer;
    ULONG returnLength;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenAppContainerSid,
        tokenAppContainerSid,
        sizeof(tokenAppContainerSidBuffer),
        &returnLength
        );

    if (NT_SUCCESS(status))
    {
        if (tokenAppContainerSid->TokenAppContainer)
        {
            *AppContainerSid = PhAllocateCopy(tokenAppContainerSid->TokenAppContainer, PhLengthSid(tokenAppContainerSid->TokenAppContainer));
        }
        else
        {
            status = STATUS_NOT_FOUND;
        }
    }

    return status;
}

NTSTATUS PhGetTokenAppContainerSid(
    _In_ HANDLE TokenHandle,
    _Out_ PPH_TOKEN_APPCONTAINER AppContainerSid
    )
{
    NTSTATUS status;
    ULONG returnLength;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenAppContainerSid,
        AppContainerSid,
        sizeof(PH_TOKEN_APPCONTAINER),
        &returnLength
        );

    if (NT_SUCCESS(status) && !AppContainerSid->AppContainer.Sid)
    {
        status = STATUS_NOT_FOUND;
    }

    return status;
}

NTSTATUS PhGetTokenSecurityAttributes(
    _In_ HANDLE TokenHandle,
    _Out_ PTOKEN_SECURITY_ATTRIBUTES_INFORMATION* SecurityAttributes
    )
{
    return PhpQueryTokenVariableSize(
        TokenHandle,
        TokenSecurityAttributes,
        SecurityAttributes
        );
}

NTSTATUS PhGetTokenSecurityAttribute(
    _In_ HANDLE TokenHandle,
    _In_ PPH_STRINGREF AttributeName,
    _Out_ PTOKEN_SECURITY_ATTRIBUTES_INFORMATION* SecurityAttributes
    )
{
    NTSTATUS status;
    UNICODE_STRING attributeName;
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION buffer;
    ULONG bufferLength;
    ULONG returnLength;

    if (!PhStringRefToUnicodeString(AttributeName, &attributeName))
        return STATUS_NAME_TOO_LONG;

    returnLength = 0;
    bufferLength = 0x200;
    buffer = PhAllocate(bufferLength);
    memset(buffer, 0, bufferLength);

    status = NtQuerySecurityAttributesToken(
        TokenHandle,
        &attributeName,
        1,
        buffer,
        bufferLength,
        &returnLength
        );

    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
    {
        bufferLength = returnLength;
        buffer = PhAllocate(bufferLength);
        memset(buffer, 0, bufferLength);

        status = NtQuerySecurityAttributesToken(
            TokenHandle,
            &attributeName,
            1,
            buffer,
            bufferLength,
            &returnLength
            );
    }

    if (NT_SUCCESS(status))
    {
        if (returnLength == sizeof(TOKEN_SECURITY_ATTRIBUTES_INFORMATION))
        {
            PhFree(buffer);
            return STATUS_NOT_FOUND;
        }

        *SecurityAttributes = buffer;
    }
    else
    {
        PhFree(buffer);
    }

    return status;
}

BOOLEAN PhDoesTokenSecurityAttributeExist(
    _In_ HANDLE TokenHandle,
    _In_ PPH_STRINGREF AttributeName
    )
{
    NTSTATUS status;
    UNICODE_STRING attributeName;
    ULONG returnLength;

    if (!PhStringRefToUnicodeString(AttributeName, &attributeName))
        return FALSE;

    status = NtQuerySecurityAttributesToken(
        TokenHandle,
        &attributeName,
        1,
        NULL,
        0,
        &returnLength
        );

    if (status == STATUS_BUFFER_TOO_SMALL)
        return TRUE;

    return FALSE;
}

PTOKEN_SECURITY_ATTRIBUTE_V1 PhFindTokenSecurityAttributeName(
    _In_ PTOKEN_SECURITY_ATTRIBUTES_INFORMATION Attributes,
    _In_ PPH_STRINGREF AttributeName
    )
{
    for (ULONG i = 0; i < Attributes->AttributeCount; i++)
    {
        PTOKEN_SECURITY_ATTRIBUTE_V1 attribute = &Attributes->Attribute.pAttributeV1[i];
        PH_STRINGREF attributeName;

        PhUnicodeStringToStringRef(&attribute->Name, &attributeName);

        if (PhEqualStringRef(&attributeName, AttributeName, FALSE))
        {
            return attribute;
        }
    }

    return NULL;
}

BOOLEAN PhGetTokenIsFullTrustPackage(
    _In_ HANDLE TokenHandle
    )
{
    static PH_STRINGREF attributeName = PH_STRINGREF_INIT(L"WIN://SYSAPPID");
    BOOLEAN tokenIsAppContainer = FALSE;

    if (NT_SUCCESS(PhDoesTokenSecurityAttributeExist(TokenHandle, &attributeName)))
    {
        if (NT_SUCCESS(PhGetTokenIsAppContainer(TokenHandle, &tokenIsAppContainer)))
        {
            if (tokenIsAppContainer)
                return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

NTSTATUS PhGetProcessIsStronglyNamed(
    _In_ HANDLE ProcessHandle,
    _Out_ PBOOLEAN IsStronglyNamed
    )
{
    NTSTATUS status;
    PROCESS_EXTENDED_BASIC_INFORMATION basicInfo;

    status = PhGetProcessExtendedBasicInformation(ProcessHandle, &basicInfo);

    if (NT_SUCCESS(status))
    {
        *IsStronglyNamed = !!basicInfo.IsStronglyNamed;
    }

    return status;
}

BOOLEAN PhGetProcessIsFullTrustPackage(
    _In_ HANDLE ProcessHandle
    )
{
    BOOLEAN processIsStronglyNamed = FALSE;
    BOOLEAN tokenIsAppContainer = FALSE;

    if (NT_SUCCESS(PhGetProcessIsStronglyNamed(ProcessHandle, &processIsStronglyNamed)))
    {
        if (processIsStronglyNamed)
        {
            HANDLE tokenHandle;

            if (NT_SUCCESS(PhOpenProcessToken(ProcessHandle, TOKEN_QUERY, &tokenHandle)))
            {
                PhGetTokenIsAppContainer(tokenHandle, &tokenIsAppContainer);
                NtClose(tokenHandle);
            }

            if (tokenIsAppContainer)
                return FALSE;

            return TRUE;
        }
    }

    return FALSE;
}

// rev from PackageIdFromFullName (dmex)
PPH_STRING PhGetProcessPackageFullName(
    _In_ HANDLE ProcessHandle
    )
{
    HANDLE tokenHandle;
    PPH_STRING packageName = NULL;

    if (NT_SUCCESS(PhOpenProcessToken(ProcessHandle, TOKEN_QUERY, &tokenHandle)))
    {
        packageName = PhGetTokenPackageFullName(tokenHandle);
        NtClose(tokenHandle);
    }

    return packageName;
}

NTSTATUS PhGetTokenIsLessPrivilegedAppContainer(
    _In_ HANDLE TokenHandle,
    _Out_ PBOOLEAN IsLessPrivilegedAppContainer
    )
{
    static PH_STRINGREF attributeName = PH_STRINGREF_INIT(L"WIN://NOALLAPPPKG");

    if (PhDoesTokenSecurityAttributeExist(TokenHandle, &attributeName))
        *IsLessPrivilegedAppContainer = TRUE;
    else
        *IsLessPrivilegedAppContainer = FALSE;

    // TODO: NtQueryInformationToken(TokenIsLessPrivilegedAppContainer);

    return STATUS_SUCCESS;
}

ULONG64 PhGetTokenSecurityAttributeValueUlong64(
    _In_ HANDLE TokenHandle,
    _In_ PPH_STRINGREF Name,
    _In_ ULONG ValueIndex
    )
{
    ULONG64 value = MAXULONG64;
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION info;

    if (NT_SUCCESS(PhGetTokenSecurityAttribute(TokenHandle, Name, &info)))
    {
        PTOKEN_SECURITY_ATTRIBUTE_V1 attribute = PhFindTokenSecurityAttributeName(info, Name);

        if (attribute && attribute->ValueType == TOKEN_SECURITY_ATTRIBUTE_TYPE_UINT64 && ValueIndex < attribute->ValueCount)
        {
            value = attribute->Values.pUint64[ValueIndex];
        }

        PhFree(info);
    }

    return value;
}

PPH_STRING PhGetTokenSecurityAttributeValueString(
    _In_ HANDLE TokenHandle,
    _In_ PPH_STRINGREF Name,
    _In_ ULONG ValueIndex
    )
{
    PPH_STRING value = NULL;
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION info;

    if (NT_SUCCESS(PhGetTokenSecurityAttribute(TokenHandle, Name, &info)))
    {
        PTOKEN_SECURITY_ATTRIBUTE_V1 attribute = PhFindTokenSecurityAttributeName(info, Name);

        if (attribute && attribute->ValueType == TOKEN_SECURITY_ATTRIBUTE_TYPE_STRING && ValueIndex < attribute->ValueCount)
        {
            value = PhCreateStringFromUnicodeString(&attribute->Values.pString[ValueIndex]);
        }

        PhFree(info);
    }

    return value;
}

// rev from GetApplicationUserModelId/GetApplicationUserModelIdFromToken (dmex)
PPH_STRING PhGetTokenPackageApplicationUserModelId(
    _In_ HANDLE TokenHandle
    )
{
    static PH_STRINGREF attributeName = PH_STRINGREF_INIT(L"WIN://SYSAPPID");
    static PH_STRINGREF seperator = PH_STRINGREF_INIT(L"!");
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION info;
    PPH_STRING applicationUserModelId = NULL;

    if (NT_SUCCESS(PhGetTokenSecurityAttribute(TokenHandle, &attributeName, &info)))
    {
        PTOKEN_SECURITY_ATTRIBUTE_V1 attribute = PhFindTokenSecurityAttributeName(info, &attributeName);

        if (attribute && attribute->ValueType == TOKEN_SECURITY_ATTRIBUTE_TYPE_STRING && attribute->ValueCount >= 3)
        {
            PPH_STRING relativeIdName;
            PPH_STRING packageFamilyName;

            relativeIdName = PhCreateStringFromUnicodeString(&attribute->Values.pString[1]);
            packageFamilyName = PhCreateStringFromUnicodeString(&attribute->Values.pString[2]);

            applicationUserModelId = PhConcatStringRef3(
                &packageFamilyName->sr,
                &seperator,
                &relativeIdName->sr
                );

            PhDereferenceObject(packageFamilyName);
            PhDereferenceObject(relativeIdName);
        }

        PhFree(info);
    }

    return applicationUserModelId;
}

PPH_STRING PhGetTokenPackageFullName(
    _In_ HANDLE TokenHandle
    )
{
    static PH_STRINGREF attributeName = PH_STRINGREF_INIT(L"WIN://SYSAPPID");
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION info;
    PPH_STRING packageFullName = NULL;

    if (NT_SUCCESS(PhGetTokenSecurityAttribute(TokenHandle, &attributeName, &info)))
    {
        PTOKEN_SECURITY_ATTRIBUTE_V1 attribute = PhFindTokenSecurityAttributeName(info, &attributeName);

        if (attribute && attribute->ValueType == TOKEN_SECURITY_ATTRIBUTE_TYPE_STRING)
        {
            packageFullName = PhCreateStringFromUnicodeString(&attribute->Values.pString[0]);
        }

        PhFree(info);
    }

    return packageFullName;
}

NTSTATUS PhGetTokenNamedObjectPath(
    _In_ HANDLE TokenHandle,
    _In_opt_ PSID Sid,
    _Out_ PPH_STRING* ObjectPath
    )
{
    NTSTATUS status;
    UNICODE_STRING objectPath;

    if (!RtlGetTokenNamedObjectPath_Import())
        return STATUS_NOT_SUPPORTED;

    RtlInitEmptyUnicodeString(&objectPath, NULL, 0);

    status = RtlGetTokenNamedObjectPath_Import()(
        TokenHandle,
        Sid,
        &objectPath
        );

    if (NT_SUCCESS(status))
    {
        *ObjectPath = PhCreateStringFromUnicodeString(&objectPath);
        RtlFreeUnicodeString(&objectPath);
    }

    return status;
}

NTSTATUS PhGetAppContainerNamedObjectPath(
    _In_ HANDLE TokenHandle,
    _In_opt_ PSID AppContainerSid,
    _In_ BOOLEAN RelativePath,
    _Out_ PPH_STRING* ObjectPath
    )
{
    NTSTATUS status;
    UNICODE_STRING objectPath;

    if (!RtlGetAppContainerNamedObjectPath_Import())
        return STATUS_UNSUCCESSFUL;

    RtlInitEmptyUnicodeString(&objectPath, NULL, 0);

    status = RtlGetAppContainerNamedObjectPath_Import()(
        TokenHandle,
        AppContainerSid,
        RelativePath,
        &objectPath
        );

    if (NT_SUCCESS(status))
    {
        *ObjectPath = PhCreateStringFromUnicodeString(&objectPath);
        RtlFreeUnicodeString(&objectPath);
    }

    return status;
}

BOOLEAN PhPrivilegeCheck(
    _In_ HANDLE TokenHandle,
    _In_ ULONG Privilege
    )
{
    CHAR privilegesBuffer[FIELD_OFFSET(PRIVILEGE_SET, Privilege) + sizeof(LUID_AND_ATTRIBUTES) * 1];
    PPRIVILEGE_SET requiredPrivileges;
    BOOLEAN result = FALSE;

    requiredPrivileges = (PPRIVILEGE_SET)privilegesBuffer;
    requiredPrivileges->PrivilegeCount = 1;
    requiredPrivileges->Control = PRIVILEGE_SET_ALL_NECESSARY;
    requiredPrivileges->Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    requiredPrivileges->Privilege[0].Luid = RtlConvertUlongToLuid(Privilege);

    NtPrivilegeCheck(TokenHandle, requiredPrivileges, &result);

    return result;
}

BOOLEAN PhPrivilegeCheckAny(
    _In_ HANDLE TokenHandle,
    _In_ ULONG Privilege
    )
{
    CHAR privilegesBuffer[FIELD_OFFSET(PRIVILEGE_SET, Privilege) + sizeof(LUID_AND_ATTRIBUTES) * 1];
    PPRIVILEGE_SET requiredPrivileges;
    BOOLEAN result = FALSE;

    requiredPrivileges = (PPRIVILEGE_SET)privilegesBuffer;
    requiredPrivileges->PrivilegeCount = 1;
    requiredPrivileges->Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    requiredPrivileges->Privilege[0].Luid = RtlConvertUlongToLuid(Privilege);

    NtPrivilegeCheck(TokenHandle, requiredPrivileges, &result);

    if (requiredPrivileges->Privilege[0].Attributes == SE_PRIVILEGE_USED_FOR_ACCESS)
        return TRUE;

    return FALSE;
}

/**
 * Modifies a token privilege.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_ADJUST_PRIVILEGES access.
 * \param PrivilegeName The name of the privilege to modify. If this parameter is NULL, you must
 * specify a LUID in the \a PrivilegeLuid parameter.
 * \param PrivilegeLuid The LUID of the privilege to modify. If this parameter is NULL, you must
 * specify a name in the \a PrivilegeName parameter.
 * \param Attributes The new attributes of the privilege.
 */
BOOLEAN PhSetTokenPrivilege(
    _In_ HANDLE TokenHandle,
    _In_opt_ PWSTR PrivilegeName,
    _In_opt_ PLUID PrivilegeLuid,
    _In_ ULONG Attributes
    )
{
    NTSTATUS status;
    TOKEN_PRIVILEGES privileges;

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = Attributes;

    if (PrivilegeLuid)
    {
        privileges.Privileges[0].Luid = *PrivilegeLuid;
    }
    else if (PrivilegeName)
    {
        PH_STRINGREF privilegeName;

        PhInitializeStringRefLongHint(&privilegeName, PrivilegeName);

        if (!PhLookupPrivilegeValue(
            &privilegeName,
            &privileges.Privileges[0].Luid
            ))
            return FALSE;
    }
    else
    {
        return FALSE;
    }

    if (!NT_SUCCESS(status = NtAdjustPrivilegesToken(
        TokenHandle,
        FALSE,
        &privileges,
        0,
        NULL,
        NULL
        )))
        return FALSE;

    if (status == STATUS_NOT_ALL_ASSIGNED)
        return FALSE;

    return TRUE;
}

BOOLEAN PhSetTokenPrivilege2(
    _In_ HANDLE TokenHandle,
    _In_ LONG Privilege,
    _In_ ULONG Attributes
    )
{
    LUID privilegeLuid;

    privilegeLuid = RtlConvertLongToLuid(Privilege);

    return PhSetTokenPrivilege(TokenHandle, NULL, &privilegeLuid, Attributes);
}

NTSTATUS PhAdjustPrivilege(
    _In_opt_ PWSTR PrivilegeName,
    _In_opt_ LONG Privilege,
    _In_ BOOLEAN Enable
    )
{
    NTSTATUS status;
    HANDLE tokenHandle;
    TOKEN_PRIVILEGES privileges;

    status = NtOpenProcessToken(
        NtCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES,
        &tokenHandle
        );

    if (!NT_SUCCESS(status))
        return status;

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    if (Privilege)
    {
        LUID privilegeLuid;

        privilegeLuid = RtlConvertLongToLuid(Privilege);

        privileges.Privileges[0].Luid = privilegeLuid;
    }
    else if (PrivilegeName)
    {
        PH_STRINGREF privilegeName;

        PhInitializeStringRefLongHint(&privilegeName, PrivilegeName);

        if (!PhLookupPrivilegeValue(
            &privilegeName,
            &privileges.Privileges[0].Luid
            ))
        {
            NtClose(tokenHandle);
            return STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        NtClose(tokenHandle);
        return STATUS_INVALID_PARAMETER_1;
    }

    status = NtAdjustPrivilegesToken(
        tokenHandle,
        FALSE,
        &privileges,
        0,
        NULL,
        NULL
        );

    NtClose(tokenHandle);

    if (status == STATUS_NOT_ALL_ASSIGNED)
        return STATUS_PRIVILEGE_NOT_HELD;

    return status;
}

/**
* Modifies a token group.
*
* \param TokenHandle A handle to a token. The handle must have TOKEN_ADJUST_GROUPS access.
* \param GroupName The name of the group to modify. If this parameter is NULL, you must
* specify a PSID in the \a GroupSid parameter.
* \param GroupSid The PSID of the group to modify. If this parameter is NULL, you must
* specify a group name in the \a GroupName parameter.
* \param Attributes The new attributes of the group.
*/
NTSTATUS PhSetTokenGroups(
    _In_ HANDLE TokenHandle,
    _In_opt_ PWSTR GroupName,
    _In_opt_ PSID GroupSid,
    _In_ ULONG Attributes
    )
{
    NTSTATUS status;
    TOKEN_GROUPS groups;

    groups.GroupCount = 1;
    groups.Groups[0].Attributes = Attributes;

    if (GroupSid)
    {
        groups.Groups[0].Sid = GroupSid;
    }
    else if (GroupName)
    {
        PH_STRINGREF groupName;

        PhInitializeStringRefLongHint(&groupName, GroupName);

        if (!NT_SUCCESS(status = PhLookupName(&groupName, &groups.Groups[0].Sid, NULL, NULL)))
            return status;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }

    status = NtAdjustGroupsToken(
        TokenHandle,
        FALSE,
        &groups,
        0,
        NULL,
        NULL
        );

    if (GroupName && groups.Groups[0].Sid)
        PhFree(groups.Groups[0].Sid);

    return status;
}

NTSTATUS PhSetTokenSessionId(
    _In_ HANDLE TokenHandle,
    _In_ ULONG SessionId
    )
{
    return NtSetInformationToken(
        TokenHandle,
        TokenSessionId,
        &SessionId,
        sizeof(ULONG)
        );
}

/**
 * Sets whether virtualization is enabled for a token.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_WRITE access.
 * \param IsVirtualizationEnabled A boolean indicating whether virtualization is to be enabled for
 * the token.
 */
NTSTATUS PhSetTokenIsVirtualizationEnabled(
    _In_ HANDLE TokenHandle,
    _In_ BOOLEAN IsVirtualizationEnabled
    )
{
    ULONG virtualizationEnabled;

    virtualizationEnabled = IsVirtualizationEnabled;

    return NtSetInformationToken(
        TokenHandle,
        TokenVirtualizationEnabled,
        &virtualizationEnabled,
        sizeof(ULONG)
        );
}

/**
* Gets a token's integrity level RID. Can handle custom integrity levels.
*
* \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
* \param IntegrityLevelRID A variable which receives the integrity level of the token.
* \param IntegrityString A variable which receives a pointer to a string containing a string
* representation of the integrity level.
*/
NTSTATUS PhGetTokenIntegrityLevelRID(
    _In_ HANDLE TokenHandle,
    _Out_opt_ PMANDATORY_LEVEL_RID IntegrityLevelRID,
    _Out_opt_ PWSTR *IntegrityString
    )
{
    NTSTATUS status;
    UCHAR mandatoryLabelBuffer[TOKEN_INTEGRITY_LEVEL_MAX_SIZE];
    PTOKEN_MANDATORY_LABEL mandatoryLabel = (PTOKEN_MANDATORY_LABEL)mandatoryLabelBuffer;
    ULONG returnLength;
    ULONG subAuthoritiesCount;
    ULONG subAuthority;
    PWSTR integrityString;
    BOOLEAN tokenIsAppContainer;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenIntegrityLevel,
        mandatoryLabel,
        sizeof(mandatoryLabelBuffer),
        &returnLength
        );

    if (!NT_SUCCESS(status))
        return status;

    subAuthoritiesCount = *PhSubAuthorityCountSid(mandatoryLabel->Label.Sid);

    if (subAuthoritiesCount > 0)
    {
        subAuthority = *PhSubAuthoritySid(mandatoryLabel->Label.Sid, subAuthoritiesCount - 1);
    }
    else
    {
        subAuthority = SECURITY_MANDATORY_UNTRUSTED_RID;
    }

    if (IntegrityString)
    {
        if (NT_SUCCESS(PhGetTokenIsAppContainer(TokenHandle, &tokenIsAppContainer)) && tokenIsAppContainer)
        {
            integrityString = L"AppContainer";
        }
        else
        {
            switch (subAuthority)
            {
            case SECURITY_MANDATORY_UNTRUSTED_RID:
                integrityString = L"Untrusted";
                break;
            case SECURITY_MANDATORY_LOW_RID:
                integrityString = L"Low";
                break;
            case SECURITY_MANDATORY_MEDIUM_RID:
                integrityString = L"Medium";
                break;
            case SECURITY_MANDATORY_MEDIUM_PLUS_RID:
                integrityString = L"Medium +";
                break;
            case SECURITY_MANDATORY_HIGH_RID:
                integrityString = L"High";
                break;
            case SECURITY_MANDATORY_SYSTEM_RID:
                integrityString = L"System";
                break;
            case SECURITY_MANDATORY_PROTECTED_PROCESS_RID:
                integrityString = L"Protected";
                break;
            default:
                integrityString = L"Other";
                break;
            }
        }

        *IntegrityString = integrityString;
    }

    if (IntegrityLevelRID)
        *IntegrityLevelRID = subAuthority;

    return status;
}

/**
 * Gets a token's integrity level.
 *
 * \param TokenHandle A handle to a token. The handle must have TOKEN_QUERY access.
 * \param IntegrityLevel A variable which receives the integrity level of the token.
 * If the integrity level is not a well-known one the function fails.
 * \param IntegrityString A variable which receives a pointer to a string containing a string
 * representation of the integrity level.
 */
NTSTATUS PhGetTokenIntegrityLevel(
    _In_ HANDLE TokenHandle,
    _Out_opt_ PMANDATORY_LEVEL IntegrityLevel,
    _Out_opt_ PWSTR *IntegrityString
    )
{
    NTSTATUS status;
    MANDATORY_LEVEL_RID integrityLevelRID;
    MANDATORY_LEVEL integrityLevel;

    status = PhGetTokenIntegrityLevelRID(TokenHandle, &integrityLevelRID, IntegrityString);

    if (!NT_SUCCESS(status))
        return status;

    if (IntegrityLevel)
    {
        switch (integrityLevelRID)
        {
        case SECURITY_MANDATORY_UNTRUSTED_RID:
            integrityLevel = MandatoryLevelUntrusted;
            break;
        case SECURITY_MANDATORY_LOW_RID:
            integrityLevel = MandatoryLevelLow;
            break;
        case SECURITY_MANDATORY_MEDIUM_RID:
            integrityLevel = MandatoryLevelMedium;
            break;
        case SECURITY_MANDATORY_MEDIUM_PLUS_RID:
            integrityLevel = MandatoryLevelMedium;
            break;
        case SECURITY_MANDATORY_HIGH_RID:
            integrityLevel = MandatoryLevelHigh;
            break;
        case SECURITY_MANDATORY_SYSTEM_RID:
            integrityLevel = MandatoryLevelSystem;
            break;
        case SECURITY_MANDATORY_PROTECTED_PROCESS_RID:
            integrityLevel = MandatoryLevelSecureProcess;
            break;
        default:
            return STATUS_UNSUCCESSFUL;
        }

        *IntegrityLevel = integrityLevel;
    }

    return status;
}

NTSTATUS PhCreateImpersonationToken(
    _In_ HANDLE ThreadHandle,
    _Out_ PHANDLE TokenHandle
    )
{
    NTSTATUS status;
    HANDLE tokenHandle;
    SECURITY_QUALITY_OF_SERVICE securityService;

    status = PhRevertImpersonationToken(ThreadHandle);

    if (!NT_SUCCESS(status))
        return status;

    securityService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    securityService.ImpersonationLevel = SecurityImpersonation;
    securityService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    securityService.EffectiveOnly = FALSE;

    status = NtImpersonateThread(
        ThreadHandle,
        ThreadHandle,
        &securityService
        );

    if (!NT_SUCCESS(status))
        return status;

    status = PhOpenThreadToken(
        ThreadHandle,
        TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
        FALSE,
        &tokenHandle
        );

    if (NT_SUCCESS(status))
    {
        *TokenHandle = tokenHandle;
    }

    return status;
}

NTSTATUS PhImpersonateToken(
    _In_ HANDLE ThreadHandle,
    _In_ HANDLE TokenHandle
    )
{
    NTSTATUS status;
    TOKEN_TYPE tokenType;
    ULONG returnLength;

    status = NtQueryInformationToken(
        TokenHandle,
        TokenType,
        &tokenType,
        sizeof(TOKEN_TYPE),
        &returnLength
        );

    if (!NT_SUCCESS(status))
        return status;

    if (tokenType == TokenPrimary)
    {
        SECURITY_QUALITY_OF_SERVICE securityService;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE tokenHandle;

        InitializeObjectAttributes(
            &objectAttributes,
            NULL,
            0,
            NULL,
            NULL
            );

        securityService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        securityService.ImpersonationLevel = SecurityImpersonation;
        securityService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        securityService.EffectiveOnly = FALSE;
        objectAttributes.SecurityQualityOfService = &securityService;

        status = NtDuplicateToken(
            TokenHandle,
            TOKEN_IMPERSONATE | TOKEN_QUERY,
            &objectAttributes,
            FALSE,
            TokenImpersonation,
            &tokenHandle
            );

        if (!NT_SUCCESS(status))
            return status;

        status = NtSetInformationThread(
            ThreadHandle,
            ThreadImpersonationToken,
            &tokenHandle,
            sizeof(HANDLE)
            );

        NtClose(tokenHandle);
    }
    else
    {
        status = NtSetInformationThread(
            ThreadHandle,
            ThreadImpersonationToken,
            &TokenHandle,
            sizeof(HANDLE)
            );
    }

    return status;
}

NTSTATUS PhRevertImpersonationToken(
    _In_ HANDLE ThreadHandle
    )
{
    HANDLE tokenHandle = NULL;

    return NtSetInformationThread(
        ThreadHandle,
        ThreadImpersonationToken,
        &tokenHandle,
        sizeof(HANDLE)
        );
}
