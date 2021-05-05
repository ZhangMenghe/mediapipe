#ifndef FACE_ACU_GENERATOR_H
#define FACE_ACU_GENERATOR_H

#define avg(X,Y) (X+Y)*0.5

#include "mediapipe/framework/port/opencv_core_inc.h"
#include <map>
#include <algorithm> 
#include <locale>         // std::locale, std::isspace
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <glm/glm.hpp>
#include <limits>
#include <glm/gtx/string_cast.hpp>
#include <PrimeRenderer/PointCloudRenderer.h>
#include <PrimeRenderer/QuadRenderer.h>

#include <chrono> 

namespace mediapipe {
static bool my_isspace(char ch){/*return ch ==' ';*/return std::isspace(static_cast<unsigned char>(ch));}
const float MINFINITY = std::numeric_limits<float>::max();

struct faceRect{
	float centerX, centerY;
	float width, height;
	float rotation;
	bool normalized;
	faceRect(){}
	faceRect(float cx, float cy, float w, float h, float r, bool bn){
		centerX=cx;centerY=cy;width=w;height=h;rotation=r;normalized=bn;
	}
};
struct acuPoint{
	std::string channel;
	int id;
	std::string name;
	std::string region;
	std::string dx, dy;
	glm::vec3 p1, p2;
	bool symmetry;
	bool draw;
	acuPoint(){
		symmetry = false;draw=false;
	}
	acuPoint(std::string channel_, int id_, std::string n_, std::string region_, std::string dx_, std::string dy_, std::string is_sym){
		channel = channel_;id=id_;name=n_;dx = dx_; dy=dy_; region=region_;
		dx.erase(std::remove_if( dx.begin(),  dx.end(), my_isspace),  dx.end());
		dy.erase(std::remove_if( dy.begin(),  dy.end(), my_isspace),  dy.end());
		symmetry = (is_sym == "TRUE" || is_sym=="True");
		draw = false;
	}
};

class acuGenerator{
private:
    std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
    static acuGenerator* myPtr_;

	const int ACU_INFO_NUMS = 9;
	const std::vector<std::string> unit_functions{"GetX","GetY","Avg"};
	std::string spath;

	// std::vector<std::string> channels_;
	std::map<std::string,acuPoint> acu_ref_map, acu_map;
	std::unordered_map<std::string, std::vector<unsigned short>> meridian_map;
	

	// const float * ptr = nullptr;
	float unit_size = .0f;
	float unit_hair_size = .0f;
	bool draw_ref=false;
	bool draw_all_points = false;
	bool draw_acu_points = true;
	std::set<std::string> target_channels=std::set<std::string>{"DU"};
	std::unordered_map<std::string, int> meridian_num_map;

	float* pdata_ = nullptr;
	unsigned short* pind = nullptr;
	int data_num = 0;
	float cos_theta, sin_theta;
	glm::mat2 R, R_prime;
	glm::vec2 mps[468];

	PointRenderer* prenderer;
	std::vector<QuadRenderer*> m_quad_renderers;
	std::unordered_map<std::string, PointRenderer*> line_renderers;

	void on_process(std::map<std::string,acuPoint>& mp, bool sel_channel=false);
    void setup_shader_content();
	void read_from_csv();
	float calculate_from_string(std::string str);
	bool getXY(std::string content, float& r1, float& r2, int& pid);
	bool get_avg_value(std::string content, float& r1, float& r2, int& pid);
	std::vector<float> process_line(std::string content);
	float getGLPos(float p){return p*2.0f-1.0f;}

	void gen_mapped_points(std::map<std::string,acuPoint> mp, int& num);
	void gen_all_points(const float* points,int& data_num);
	bool cal_unit_size(cv::Mat hair_mask, const float* points);
public:
    static acuGenerator* instance();
	void clearTarget(){target_channels.clear();}
	void addTargetMerdian(std::string target_meridian){
		if(target_meridian.compare("Refs")==0)draw_ref = true;
		else target_channels.insert(target_meridian);
	}
	void removeTargetMerdian(std::string target_meridian){
		if(target_meridian.compare("Refs")==0)draw_ref = false;
		else target_channels.erase(target_meridian);
	}

	void onSetup(std::string shader_path);
    void onDraw(faceRect rect, cv::Mat& hair_mask, std::vector<faceRect> ear_rects, const float* points);
    void onDestroy();
};
}
#endif