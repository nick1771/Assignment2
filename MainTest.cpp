#include <string_view>
#include <iostream>

#include "BackgroundService.h"

using namespace BackgroundService;

void assertTrue(bool result, std::string_view name) {
    if (!result) {
        std::cout << "Test failed [" << name << "]\n";
    } else {
        std::cout << "Test success [" << name << "]\n";
    }
}

void testParseBroadcastDeviceList() {
    auto deviceList = parseBroadcastDeviceList("M[0X:13:12:32,102.103.133.13]M");

    assertTrue(deviceList.size() == 1, "Device list size 1");
    assertTrue(deviceList[0].ip == "102.103.133.13", "Ip equals 102.103.133.13");
    assertTrue(deviceList[0].mac == "0X:13:12:32", "Mac equals 102.103.133.13");

    auto deviceList2 = parseBroadcastDeviceList("M[0X:13:12:32,");

    assertTrue(deviceList2.size() == 0, "Device list size 0 from partial data");
}

void shouldRemoveExpiredDevices() {
    auto existingDevices = parseBroadcastDeviceList("M[1,1]M");
    existingDevices[0].timeToLive = 2.f;

    updateDeviceInformation(existingDevices, {}, {});
    assertTrue(existingDevices.size() == 1, "Size 1 after update");

    updateDeviceInformation(existingDevices, {}, {});
    assertTrue(existingDevices.size() == 0, "Size 0 after update");
}

void shouldRemoveExpiredDevicesAndAddNew() {
    auto existingDevices2 = parseBroadcastDeviceList("M[1,1]M");
    existingDevices2[0].timeToLive = 1.f;

    auto newDevice = parseBroadcastDeviceList("M[2,2]M");
    updateDeviceInformation(existingDevices2, newDevice, {});
 
    assertTrue(existingDevices2.size() == 1, "Size 1 after update");
    assertTrue(existingDevices2[0].ip == "2", "Ip 2 after update");    
}

void shouldUpdateTimeToLiveForExisting() {
    auto existingDevices2 = parseBroadcastDeviceList("M[1,1]M");
    existingDevices2[0].timeToLive = 2.f;

    auto newDevice = parseBroadcastDeviceList("M[1,1]M");
    updateDeviceInformation(existingDevices2, newDevice, {});

    assertTrue(existingDevices2.size() == 1, "Size 1 after update");
    assertTrue(existingDevices2[0].ip == "1", "Ip 1 after update");
    assertTrue(existingDevices2[0].timeToLive == 30.f, "Time to live 30 after update");  
}

void shouldUpdateTimeToLiveForExisting2() {
    auto existingDevices2 = parseBroadcastDeviceList("M[1,1]M");
    existingDevices2[0].timeToLive = 1.f;

    auto newDevice = parseBroadcastDeviceList("M[1,1]M");
    updateDeviceInformation(existingDevices2, newDevice, {});

    assertTrue(existingDevices2.size() == 1, "Size 1 after update 2");
    assertTrue(existingDevices2[0].ip == "1", "Ip 1 after update 2");
    assertTrue(existingDevices2[0].timeToLive == 30.f, "Time to live 30 after update 2");  
}

void shouldAddNewDevices() {
    auto existingDevices2 = parseBroadcastDeviceList("M[1,1]M");

    auto newDevices = parseBroadcastDeviceList("M[2,2]MM[3,3]M");
    updateDeviceInformation(existingDevices2, newDevices, {});

    assertTrue(existingDevices2.size() == 3, "Size 3 after update");
    assertTrue(existingDevices2[0].timeToLive == 29.f, "Time to live 29 after update for first");
    assertTrue(existingDevices2[1].ip == "2", "Ip 2 after update for second");  
    assertTrue(existingDevices2[2].mac == "3", "Mac 3 after update for third");  
}

void testUpdateDeviceInformation() {
    shouldRemoveExpiredDevices();
    shouldRemoveExpiredDevicesAndAddNew();
    shouldUpdateTimeToLiveForExisting();
    shouldUpdateTimeToLiveForExisting2();
    shouldAddNewDevices();
}

int main() {
    testParseBroadcastDeviceList();
    testUpdateDeviceInformation();
}
