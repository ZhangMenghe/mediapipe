#ifndef SLAM_HELMSLEY_VR_H
#define SLAM_HELMSLEY_VR_H
#include <platforms/platform.h>
#include <vrController.h>
#include <overlayController.h>
#include <utils/dicomLoader.h>
#include <utils/uiController.h>
class helmsleyVR{
private:
    Manager manager;
    vrController* vrc;
    dicomLoader loader_;
    uiController ui_;

    std::string spath;
    void setup_shader_content();
public:
    void onSetup(std::string shader_path);
    void onSetup(std::string shader_path, std::string vs_txt, std::string frag_txt);

    void onDraw();
    void onDraw(glm::mat4 view_mat, glm::mat4 proj_mat, glm::mat4 vp_mat);
    void onDestroy();
};
#endif