#ifndef SLAM_HELMSLEY_VR_H
#define SLAM_HELMSLEY_VR_H
#include <platforms/platform.h>
#include <vrController.h>
#include <overlayController.h>
#include <utils/dicomLoader.h>
#include <utils/uiController.h>
#include <thread>
#include <chrono>

#ifdef RPC_ENABLED
#include <RPCs/rpcHandler.h>
#endif

class helmsleyVR{
private:
    Manager manager_;
    vrController* vrc;
    dicomLoader loader_;
    uiController ui_;

    std::string spath;

    #ifdef RPC_ENABLED
        rpcHandler* rpc_handler;
        std::thread* rpc_thread;
    #endif
    void setup_shader_content();
public:
    void onSetup(std::string shader_path);

    void onDraw();
    void onDraw(glm::mat4 view_mat, glm::mat4 proj_mat, glm::mat4 vp_mat);
    void onDestroy();
};
#endif