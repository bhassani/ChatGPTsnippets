#include <stdio.h>
#include <Windows.h>
#include <UPnP.h>

int main() {
    HRESULT hr;
    IUPnPNAT* pUPnP = NULL;
    IStaticPortMappingCollection* pMappingCollection = NULL;
    IStaticPortMapping* pMapping = NULL;

    // Initialize COM
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("Failed to initialize COM.\n");
        return 1;
    }

    // Create an instance of the UPnP NAT object
    hr = CoCreateInstance(&CLSID_UPnPNAT, NULL, CLSCTX_INPROC_SERVER, &IID_IUPnPNAT, (void**)&pUPnP);
    if (FAILED(hr)) {
        printf("Failed to create UPnP NAT instance.\n");
        CoUninitialize();
        return 1;
    }

    // Get the collection of port mappings
    hr = IUPnPNAT_get_StaticPortMappingCollection(pUPnP, &pMappingCollection);
    if (FAILED(hr)) {
        printf("Failed to get port mapping collection.\n");
        pUPnP->lpVtbl->Release(pUPnP);
        CoUninitialize();
        return 1;
    }

    // Add a port mapping
    hr = IStaticPortMappingCollection_Add(pMappingCollection, 12345, L"TCP", 12345, L"192.168.1.100", VARIANT_TRUE, L"Test Port Mapping", &pMapping);
    if (FAILED(hr)) {
        printf("Failed to add port mapping.\n");
        pMappingCollection->lpVtbl->Release(pMappingCollection);
        pUPnP->lpVtbl->Release(pUPnP);
        CoUninitialize();
        return 1;
    }

    printf("Port mapping added successfully.\n");

    // Clean up
    pMapping->lpVtbl->Release(pMapping);
    pMappingCollection->lpVtbl->Release(pMappingCollection);
    pUPnP->lpVtbl->Release(pUPnP);
    CoUninitialize();

    return 0;
}
