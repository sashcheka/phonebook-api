#include "HealthController.hpp"

std::shared_ptr<HealthController::OutgoingResponse> HealthController::health() {
    return createResponse(Status::CODE_200, "OK");
}
