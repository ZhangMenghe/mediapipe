#include "mediapipe/calculators/slam/helmsley_vr.h"
#include "mediapipe/util/resource_util.h"
#include "mediapipe/framework/port/ret_check.h"

using namespace std;

void helmsleyVR::onSetup(std::string shader_path){
    spath = shader_path;
    vrc = new vrController;
    setup_shader_content();
	
    //setup data
    loader_.setupDCMIConfig(512,512,144,-1,-1,-1,true);
    // std::cout<<"asdfasd"<<Manager::shader_contents[dvr::SHADER_TEXTUREVOLUME_VERT]<<std::endl;
    
    //setup gl
    ui_.InitAll();
	vrc->onViewCreated(false);
	overlayController::instance()->onViewCreated();
	ui_.AddTuneParams();

	// 480, 640
	overlayController::instance()->setOverlayRect(0, 440, 53, 100, 400);
    overlayController::instance()->setOverlayRect(1, 440, 22, 100, 450);

	//load data

	if(loader_.loadData(spath+"helmsley_cached/Larry_Smarr_2016/series_23_Cor_LAVA_PRE-Amira/data", 
    spath+"helmsley_cached/Larry_Smarr_2016/series_23_Cor_LAVA_PRE-Amira/mask")){
        vrc->assembleTexture(2, 512,512,144, -1, -1, -1, loader_.getVolumeData(), loader_.getChannelNum());
		loader_.reset();
	}


    #ifdef RPC_ENABLED
        rpc_handler = new rpcHandler("localhost:23333");
        rpc_thread = new thread(&rpcHandler::Run, rpc_handler);
        rpc_handler->setManager(&manager_);
        rpc_handler->setUIController(&ui_);
        rpc_handler->setVRController(vrc);
        rpc_handler->setDataLoader(&loader_);
        std::cout<<"=========rpc====="<<std::endl;
    #else
        std::cout<<"====NO =====rpc====="<<std::endl;

	#endif
    std::cout<<"====print sth"<<std::endl;

    manager_.onViewChange(640, 480);
	vrc->onViewChange(640, 480);
	overlayController::instance()->onViewChange(640, 480);
}
void helmsleyVR::onDraw(){
    vrc->onDraw();
	// if(vrc->isDrawing()) overlayController::instance()->onDraw();
    if(Manager::new_data_available){
		Manager::new_data_available = false;
		loader_.startToAssemble(vrc);
		loader_.reset();
	}
}
void helmsleyVR::onDraw(glm::mat4 view_mat, glm::mat4 proj_mat, glm::mat4 camera_pose_col_major_){
    Manager::camera->setProjMat(proj_mat);
    Manager::camera->setViewMat(view_mat);
    // Manager::camera->updateCameraPose(camera_pose_col_major_);
    onDraw();
}

void helmsleyVR::onDestroy(){

}
void helmsleyVR::setup_shader_content(){
    const char* shader_file_names[14] = {
        "shaders/textureVolume.vert",
        "shaders/textureVolume.frag",
        "shaders/raycastVolume.vert",
        "shaders/raycastVolume.frag",
        "shaders/raycastVolume.glsl",
        "shaders/raycastCompute.glsl",
        "shaders/quad.vert",
        "shaders/quad.frag",
        "shaders/cplane.vert",
        "shaders/cplane.frag",
        "shaders/colorViz.vert",
        "shaders/colorViz.frag",
        "shaders/opaViz.vert",
        "shaders/opaViz.frag"
    };
    for(int i = 0; i<int(dvr::SHADER_END); i++){
        std::string content;
        mediapipe::GetResourceContents(spath+shader_file_names[i], &content);
        vrc->setShaderContents(dvr::SHADER_FILES (i), content);   
    }
}