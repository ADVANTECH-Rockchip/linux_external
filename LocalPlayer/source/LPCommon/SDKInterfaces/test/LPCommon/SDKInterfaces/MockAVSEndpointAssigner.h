/*
 * MockRKPEndpointAssigner.h
 *
 * Copyright 2018 Rockchip.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://www.rock-chips.com/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef LOCAL_PLAYER_SDK_RKPCOMMON_SDKINTERFACES_TEST_RKPCOMMON_SDKINTERFACES_MOCKRKPENDPOINTASSIGNER_H_
#define LOCAL_PLAYER_SDK_RKPCOMMON_SDKINTERFACES_TEST_RKPCOMMON_SDKINTERFACES_MOCKRKPENDPOINTASSIGNER_H_

#include "LPCommon/SDKInterfaces/RKPEndpointAssignerInterface.h"
#include <gmock/gmock.h>

namespace localPlayerSDK {
namespace rkpCommon {
namespace sdkInterfaces {
namespace test {

/*
 * Mock class that implements @c RKPEndpointAssignerInterface.
 */
class MockRKPEndpointAssigner : public RKPEndpointAssignerInterface {
public:
    MOCK_METHOD1(setRKPEndpoint, void(const std::string& rkpEndpoint));
};

}  // namespace test
}  // namespace sdkInterfaces
}  // namespace rkpCommon
}  // namespace localPlayerSDK

#endif  // LOCAL_PLAYER_SDK_RKPCOMMON_SDKINTERFACES_TEST_RKPCOMMON_SDKINTERFACES_MOCKRKPENDPOINTASSIGNER_H_
