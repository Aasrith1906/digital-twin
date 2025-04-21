param location string = resourceGroup().location
param iotHubName string

resource iotHub 'Microsoft.Devices/IotHubs@2021-07-02' = {
  name: iotHubName
  location: location
  sku: {
    name: 'F1'  // Free tier
    capacity: 1
  }
  properties: {
    eventHubEndpoints: {
      events: {
        retentionTimeInHours: 24
        partitionCount: 2
      }
    }
    enableFileUploadNotifications: false
  }
}

output iotHubHostname string = iotHub.properties.hostName
