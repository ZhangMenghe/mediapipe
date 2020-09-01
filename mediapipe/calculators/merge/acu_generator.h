#ifndef FACE_ACU_GENERATOR_H
#define FACE_ACU_GENERATOR_H

#define avg(X,Y) (X+Y)*0.5

#include "mediapipe/framework/port/opencv_core_inc.h"
#include <map>
#include <algorithm> 
#include <locale>         // std::locale, std::isspace
#include <sstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <limits>
#include <glm/gtx/string_cast.hpp>
#include <PrimeRenderer/PointCloudRenderer.h>

namespace mediapipe {
static bool my_isspace(char ch){/*return ch ==' ';*/return std::isspace(static_cast<unsigned char>(ch));}
const float MINFINITY = std::numeric_limits<float>::max();


struct faceRect{
	float xmin, ymin;
	float width, height;
	float rotation;
	bool normalized;
	faceRect(){}
	faceRect(float xm, float ym, float w, float h, float r, bool bn){
		xmin=xm;ymin=ym;width=w;height=h;rotation=r;normalized=bn;
	}
};
struct acuPoint{
	std::string channel;
	int id;
	std::string name;
	std::string dx, dy;
	glm::vec3 p1, p2;
	bool symmetry;
	acuPoint(){
		symmetry = false;
	}
	acuPoint(std::string channel_, int id_, std::string n_, std::string dx_, std::string dy_, std::string is_sym){
		channel = channel_;id=id_;name=n_;dx = dx_; dy=dy_;
		dx.erase(std::remove_if( dx.begin(),  dx.end(), my_isspace),  dx.end());
		dy.erase(std::remove_if( dy.begin(),  dy.end(), my_isspace),  dy.end());
		symmetry = (is_sym == "TRUE" || is_sym=="True");
	}
};

class acuGenerator{
private:
	const int ACU_INFO_NUMS = 8;
	const std::vector<std::string> unit_functions{"GetX","GetY","Avg"};
	std::string spath;

	// std::vector<std::string> channels_;
	std::map<std::string,acuPoint> acu_ref_map, acu_map;
	std::unordered_map<std::string, std::vector<unsigned short>[2]> meridian_map;
	// std::unordered_map<std::string, std::vector<unsigned short>> meridian_map;
	

	// const float * ptr = nullptr;
	float unit_size = .0f;
	bool draw_ref= false;
	bool draw_all_points = false;
	bool draw_acu_points = true;
	std::string targe_ch = "ST";
	float* pdata_ = nullptr;
	int data_num = 0;
	float cos_theta, sin_theta;
	glm::mat2 R, R_prime;
	glm::vec2 mps[468];

	PointRenderer* prenderer;
	PointRenderer* line_renderer;

	void on_process(std::map<std::string,acuPoint>& mp);
    void setup_shader_content();
	void read_from_csv();
	float calculate_from_string(std::string str);
	bool getXY(std::string content, float& r1, float& r2, int& pid);
	bool get_avg_value(std::string content, float& r1, float& r2, int& pid);
	std::vector<float> process_line(std::string content);
	float getGLPos(float p){return p*2.0f-1.0f;}

	void gen_mapped_points(std::map<std::string,acuPoint> mp, int& num, std::string sel_channel="");
	void gen_all_points(const float* points,int& data_num);
	bool cal_unit_size(cv::Mat hair_mask, const float* points);
public:
	void onSetup(std::string shader_path);
    void onDraw(faceRect rect, cv::Mat hair_mask, const float* points);
    void onDestroy();
};
}
#endif