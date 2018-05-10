//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef M2MRESOURCES_H
#define M2MRESOURCES_H

#include "m2mresource.h"
#include "mbed-client/m2minterface.h"
#include "key_config_manager.h"
#include <stdio.h>

/*
 * Executable resource demonstrates how to use executable resources.
 * Example calls unregistration once it receives a POST command.
 */
class ExecutableResource {
public:
    ExecutableResource(MbedCloudClient& cloud_client) :
        _cloud_client(cloud_client) {
        executable_object = M2MInterfaceFactory::create_object("5000");
        M2MObjectInstance* exec_inst = executable_object->create_object_instance();
        M2MResource* unreg_res = exec_inst->create_dynamic_resource("1", "unregister",
            M2MResourceInstance::STRING, false);
        unreg_res->set_operation(M2MBase::POST_ALLOWED);
        unreg_res->set_execute_function(execute_callback(this, &ExecutableResource::unregister));

        M2MResource* factory_reset_res = exec_inst->create_dynamic_resource("2", "factory_reset",
            M2MResourceInstance::STRING, false);
        factory_reset_res->set_operation(M2MBase::POST_ALLOWED);
        factory_reset_res->set_execute_function(execute_callback(this, &ExecutableResource::factory_reset));
    }

    M2MObject* get_object() {
        return executable_object;
    }

    void unregister(void *) {
        printf("Unregister resource executed\r\n");
        _cloud_client.close();
    }

    void factory_reset(void *) {
        printf("Factory reset resource executed\r\n");
        _cloud_client.close();
        kcm_status_e kcm_status = kcm_factory_reset();
        if (kcm_status != KCM_STATUS_SUCCESS) {
            printf("Failed to do factory reset - %d\n", kcm_status);
        } else {
            printf("Factory reset completed. Now restart the device\n");
        }
    }

private:
    M2MObject* executable_object;
    MbedCloudClient& _cloud_client;
};

/*
 * Example resource that demonstrates how to use value_updated callback.
 */
class WritableResource : public MbedCloudClientCallback {
public:
    WritableResource() {
        writable_object = M2MInterfaceFactory::create_object("5001");
        M2MObjectInstance* obj_inst = writable_object->create_object_instance();
        M2MResource* res = obj_inst->create_dynamic_resource("1", "writable_resource",
            M2MResourceInstance::INTEGER, false /* observable */);
        res->set_operation(M2MBase::GET_PUT_ALLOWED);
        res->set_value((uint8_t*)"0", 1);
    }

    ~WritableResource() {
    }

    M2MObject* get_object() {
        return writable_object;
    }

    // implementation of MbedCloudClientCallback
    virtual void value_updated(M2MBase *base, M2MBase::BaseType type) {
        printf("PUT Request Received!\n");
        printf("\nName :'%s',\nPath : '%s',\nType : '%d' (0 for Object, 1 for Resource), \nType : '%s'\n",
                  base->name(),
                  base->uri_path(),
                  type,
                  base->resource_type());
    }


private:
    M2MObject* writable_object;
};

/*
 * Example of observable resource.
 * increment_resource() is called from main loop.
 */
class ObservableResource {
public:
    ObservableResource(): counter(0) {
        observable_object = M2MInterfaceFactory::create_object("5002");
        M2MObjectInstance* obs_inst = observable_object->create_object_instance();
        // create resource with ID '5501', which is digital input counter
        M2MResource* obs_res = obs_inst->create_dynamic_resource("1", "observable_resource",
            M2MResourceInstance::INTEGER, true /* observable */);
        // we can read this value
        obs_res->set_operation(M2MBase::GET_ALLOWED);
        // set initial value (all values in mbed Client are buffers)
        // to be able to read this data easily in the Connector console, we'll use a string
        obs_res->set_value((uint8_t*)"0", 1);
    }

    ~ObservableResource() {
    }

    M2MObject* get_object() {
        return observable_object;
    }

    void increment_resource() {
        M2MObjectInstance* inst = observable_object->object_instance();
        M2MResource* res = inst->resource("1");
        // up counter
        counter++;
        printf("increment resource value, new value of resource is %d\n", counter);
        // serialize the value of counter as a string, and tell connector
        char buffer[20];
        int size = sprintf(buffer,"%d",counter);
        res->set_value((uint8_t*)buffer, size);
    }

private:
    M2MObject* observable_object;
    uint16_t counter;
};

#endif // M2MRESOURCES_H
