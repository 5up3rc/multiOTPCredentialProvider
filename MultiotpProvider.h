/**
* BASE CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
* Extra code provided "as is" for the multiOTP open source project
*
* @author    Andre Liechti, SysCo systemes de communication sa, <info@multiotp.net>
* @version   5.2.0.0
* @date      2018-03-11
* @since     2013
* @copyright (c) 2016-2018 SysCo systemes de communication sa
* @copyright (c) 2015-2016 ArcadeJust ("RDP only" enhancement)
* @copyright (c) 2013-2015 Last Squirrel IT
* @copyright Apache License, Version 2.0
*
*
* Change Log
*
*   2018-03-11 5.2.0.0 SysCo/al New implementation from scratch
*
*********************************************************************/

#include "MultiotpHelpers.h"
#include <windows.h>
#include <strsafe.h>
#include <new>

// Add vector support
#include <vector>

#include "MultiotpCredential.h"

#define SHIFTED 0x8000

#pragma once

class MultiotpProvider : public ICredentialProvider,
                        public ICredentialProviderSetUserArray
{
  public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void **ppv)
    {
		#pragma warning( push )
		#pragma warning( disable : 4838)
		static const QITAB qit[] =
        {
            QITABENT(MultiotpProvider, ICredentialProvider), // IID_ICredentialProvider
            QITABENT(MultiotpProvider, ICredentialProviderSetUserArray), // IID_ICredentialProviderSetUserArray
            { static_cast<int>(0) },
        };
		#pragma warning( pop ) 
		return QISearch(this, qit, riid, ppv);
    }

  public:
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(_In_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const *pcpcs);

    IFACEMETHODIMP Advise(_In_ ICredentialProviderEvents *pcpe, _In_ UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(_Out_ DWORD *pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd);

    IFACEMETHODIMP GetCredentialCount(_Out_ DWORD *pdwCount,
                                      _Out_ DWORD *pdwDefault,
                                      _Out_ BOOL *pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex,
                                   _Outptr_result_nullonfailure_ ICredentialProviderCredential **ppcpc);

    IFACEMETHODIMP SetUserArray(_In_ ICredentialProviderUserArray *users);

    friend HRESULT Multiotp_CreateInstance(_In_ REFIID riid, _Outptr_ void** ppv);

  protected:
    MultiotpProvider();
    __override ~MultiotpProvider();

  private:
    void _ReleaseEnumeratedCredentials();
    void _CreateEnumeratedCredentials();
    HRESULT _EnumerateEmpty();
    HRESULT _EnumerateCredentials();
    HRESULT _EnumerateEmptyTileCredential();
private:
    long                                    _cRef;            // Used for reference counting.
    // MultiotpCredential                      *_pCredential; // MultiotpV2Credential
	std::vector<MultiotpCredential*>        _pCredential;     // MultiotpV2Credential array
    bool                                    _fRecreateEnumeratedCredentials;
	// KERB_INTERACTIVE_UNLOCK_LOGON *         _pkiulSetSerialization; // Experimental
	// DWORD                                   _dwCredUIFlags; // Experimental
	CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
    ICredentialProviderUserArray            *_pCredProviderUserArray;

};


// Filtering out the default MS password provider
class CLMSFilter : public ICredentialProviderFilter
{
public:
	//This section contains some COM boilerplate code 

	// IUnknown 
	STDMETHOD_(ULONG, AddRef)()
	{
		return _cRef++;
	}

	STDMETHOD_(ULONG, Release)()
	{
		LONG cRef = _cRef--;
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
	{
		HRESULT hr;
		if (IID_IUnknown == riid || IID_ICredentialProviderFilter == riid)
		{
			*ppv = this;
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();
			hr = S_OK;
		}
		else
		{
			*ppv = NULL;
			hr = E_NOINTERFACE;
		}
		return hr;
	}
#pragma warning(disable:4100)



public:
	friend HRESULT CLMSFilter_CreateInstance(REFIID riid, __deref_out void** ppv);

	//Implementation of ICredentialProviderFilter 
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD
		dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders);

	IFACEMETHODIMP UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsIn,
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsOut);

protected:
	CLMSFilter();
	__override ~CLMSFilter();

private:
	LONG _cRef;
};
