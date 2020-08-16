#include "acu_generator.h"
#include <iostream>
#include <fstream>
#include "mediapipe/util/resource_util.h"

using namespace mediapipe;
float acuGenerator::calculate_from_string(std::string s) {
    // std::cout<<"calculate before: "<<s<<std::endl;
    float n = s.size(), num = 0, curRes = 0, res = 0;
    char op = '+';
    for (int i = 0; i < n; ++i) {
        char c = s[i];
        if(c == 'U'){
            num  = unit_size;
        }
        else if (c >= '0' && c <= '9') {
            int si = i+1;
            while((s[si]>='0' && s[si]<='9') || s[si] == '.')si++;
            num = std::stof(s.substr(i,si-i));
            i = si-1;
            // num = num * 10 + c - '0';
        }
        else if (c == '(') {
            int j = i, cnt = 0;
            for (; i < n; ++i) {
                if (s[i] == '(') ++cnt;
                if (s[i] == ')') --cnt;
                if (cnt == 0) break;
            }
            num = calculate_from_string(s.substr(j + 1, i - j - 1));
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || i == n - 1) {
            switch (op) {
                case '+': curRes += num; break;
                case '-': curRes -= num; break;
                case '*': curRes *= num; break;
                case '/': curRes /= num; break;
            }
            if (c == '+' || c == '-' || i == n - 1) {
                res += curRes;
                curRes = 0;
            }
            op = c;
            num = 0;
        }
    }
    // std::cout<<"calculate after: "<<res<<std::endl;

    return res;
}
bool acuGenerator::getXY(std::string content, float&r1, float&r2, int& pid){
    int tid = pid;
    while(content[tid++]!='(');
    // std::cout<<"func name :"<<tid<<"-"<<pid<<" "<<content.substr(pid, tid-pid-1)<<std::endl;
    auto func_name = content.substr(pid, tid-pid-1);
    auto it = std::find(unit_functions.begin(), unit_functions.end(), func_name);
    if(it == unit_functions.end()){
        return false;    
    }
    pid = tid-1;
    while(content[pid++] != '(');
    auto rtype = content[pid++];
    //check if rtype is valid
    int ns = pid;
    while(content[pid++]!=')');
    // std::cout<<"rtype: "<<rtype<<std::endl;
    if(rtype == '#'){
        int rid = std::stof(content.substr (ns,pid-1-ns));
        // std::cout<<"rid: "<<rid<<std::endl;
        r1 = (func_name == "GetX")? ptr[3*rid]:ptr[3*rid+1];
    }else{
        std::string key = content.substr(ns-1, pid-ns);
        // std::cout<<"getxy key: "<<key<<std::endl;
        auto mp = (key[0] == 'R')?acu_ref_map:acu_map;

        //todo: check if symmetry. Tackle with it
        r1 = (func_name == "GetX")?mp[key].p1.x:mp[key].p1.y;
        if(mp[key].symmetry) r2 = (func_name == "GetX")?mp[key].p2.x:mp[key].p2.y;
    }
    return true;
}
bool acuGenerator::get_avg_value(std::string str, float& r1, float& r2, int& pid){
    if(str.compare(pid,3,"Avg") != 0) return false;
    //find limit
    pid+=4;
    int lcn = 1, rcn = 0;
    int pd = pid;
    for(; lcn != rcn && pd<str.length(); pd++){
        if(str[pd] == '(') lcn ++;
        else if(str[pd] == ')') rcn++;
    }
    float mr1, mr2, rt;
    getXY(str, mr1, rt, pid);
    if(str[pid++]!=','){std::cerr<<"wrong format";return false;}
    getXY(str, mr2, rt, pid);
    r1 = (mr1 + mr2) * 0.5f;
    r2 = rt;
    pid = pd;
    return true;
}
std::vector<float> acuGenerator::process_line(std::string content){
    std::vector<float> rv;
    // std::cout<<"process: "<<content<<std::endl;
    float res;
    int pid=0;
    std::string cline = "";
    bool hide_sym_found = false;

    float r1, r2;
    while(pid < content.length()){
        auto c = content[pid];
        if(c == '+' || c == '-' || c == '*' || c == '/' || c== '(' || c ==')' || c=='[' || c==']'){
                cline+=c;
                pid++;
        }else if(get_avg_value(content,r1, r2, pid)){
            cline+=std::to_string(r1);
        }else{
            float r2 = MINFINITY;
            if(getXY(content, res, r2, pid)){
                if(r2 < MINFINITY){
                    hide_sym_found = true;
                    cline +="[" + std::to_string(res) + "," + std::to_string(r2) + "]";
                    // std::cout<<"hide sym found "<<cline<<std::endl;
                }else cline += std::to_string(res);
            }
            else cline+=content[pid++];
        }
    }
    // std::cout<<"generated cline: "<<cline<<std::endl;

    bool symmetry = false;
    if(cline[0] == '['){
        if(hide_sym_found){
            // std::cout<<"hide sym before: "<<cline<<std::endl;
            std::size_t sp_pos = cline.find_first_of(",");
            std::size_t fb = cline.find_first_of("]") + 1;
            std::string nl = cline.substr(0, sp_pos) 
            + cline.substr(fb, cline.length()-fb) 
            + cline.substr(sp_pos, (fb-sp_pos-1)) 
            + cline.substr(fb, cline.length()-fb) + "]";
            // std::cout<<"hide sym after: "<<nl<<std::endl;
            cline = nl;
        }
        cline = cline.substr(1, cline.length() - 2);
        symmetry = true;
    }
    
    std::stringstream s_stream(cline); //create string stream from the string
    while(s_stream.good()) {
        std::string substr;
        getline(s_stream, substr, ','); //get first string delimited by comma
        // std::cout<<"num: "<<std::to_string(calculate_from_string(substr))<<std::endl;
        rv.push_back(calculate_from_string(substr));
    }
    return rv;
}
void acuGenerator::read_from_csv(){
    std::string filename = "acu.csv";//"mediapipe/res/acu.csv";

    std::string content;
    #ifdef __ANDROID__
    mediapipe::GetResourceContents(filename, &content);
    #else
    mediapipe::GetResourceContents(spath + filename, &content);
    #endif
    // std::cout<<"load content \n "<<content<<std::endl;
    if(content.empty()) return;

    std::istringstream myFile(content);
    std::string line, colname;
    std::string val;

    std::string buff[6];
    int idx = 0;

    bool ref_finished = false;
    //skip title line
    std::getline(myFile, line);
    //process ref points
    while(std::getline(myFile, line)){
        if(line.empty() || line.length()<=6){ref_finished = true;std::getline(myFile, line);continue;}
        // Create a stringstream of the current line
        std::stringstream ss(line);
        // std::cout<<"on process : "<<line<<std::endl;
        while(ss.good() && idx < 6) {
            std::string substr;
            getline(ss, substr, ','); //get first string delimited by comma
            if(substr[0] == '\"'){
                std::string mstr = substr.substr(1, substr.length()-1);
                while(ss.good()){
                    getline(ss, substr, ',');
                        mstr+=","+substr;
                    if(mstr.back()=='\"'){mstr=mstr.substr(0, mstr.length()-1);break;}
                }
                buff[idx++] = mstr;
                // std::cerr<<mstr<<" / ";  
            }
            else buff[idx++] = substr; //std::cerr<<substr<<" / ";
        }
        if(ref_finished)acu_map[buff[0] + buff[1]] = acuPoint(buff[0],std::stoi(buff[1]), buff[3], buff[4], buff[5]);
        else acu_ref_map[buff[0] + buff[1]] = acuPoint(buff[0],std::stoi(buff[1]), buff[3], buff[4], buff[5]);
        idx = 0;
    }
}
void acuGenerator::setup_shader_content(){

}
void acuGenerator::onSetup(std::string shader_path){
    spath = shader_path;
    read_from_csv();
    prenderer = new PointRenderer();

    // for(auto p : acu_map){
    //     std::cout<<p.first<<"->"<<p.second.dx<<"        "<<p.second.dy<<std::endl;
    // }

    // //debug only
    // ptr = new float[468 * 2];
    // for(int i=0;i<468 * 2;i++)ptr[i]  = 1;
    // // memset(ptr, 1.0f, sizeof(468 * 2 * sizeof(float)));
    // on_process(acu_ref_map);
    // on_process(acu_map);
}
void acuGenerator::on_process(std::map<std::string,acuPoint>& mp){
    for(auto&p:mp){
        auto rvx = process_line(p.second.dx);
        auto rvy = process_line(p.second.dy);
        if(rvx.size()>1 || rvy.size() > 1){
            p.second.symmetry = true;
            if(rvx.size() > 1) rvy.push_back(rvy[0]);
            else rvx.push_back(rvx[0]);
            p.second.p2 = glm::vec3(rvx[1], rvy[1], .0f);
        }
        p.second.p1 = glm::vec3(rvx[0], rvy[0], .0f);
        // std::cout<<"map assign: "<<((p.second.symmetry)?"sym":"")<<" "<<p.first<<"->"<<glm::to_string(p.second.p1)<<","<<glm::to_string(p.second.p2)<<std::endl;
    }
}
void acuGenerator::gen_mapped_points(std::map<std::string,acuPoint> mp, int& idx, std::string sel_channel){
    for(auto p : mp){
        if(!sel_channel.empty())if(p.second.channel!=sel_channel) continue;

        pdata_[3*idx]  = p.second.p1.x;
        pdata_[3*idx+1]  = p.second.p1.y;
        idx++;
        if(p.second.symmetry){
            pdata_[3*idx]  = p.second.p2.x;
            pdata_[3*idx+1]  = p.second.p2.y;
            idx++;
        }
        // std::cout<<points[4*idx]<<" "<<points[4*idx+1]<<std::endl;
    }
}

/*points contains 468 vertices each with x,y,z ranging[0,1], x increase to right, y increase to bottom*/
void acuGenerator::onDraw(faceRect rect, cv::Mat hair_mask, const float* points){
    //get unit size
    
    mask = hair_mask.clone();
    auto ms = hair_mask.size();
    int ref_RHD1_ids[2]={8,9};
    float ref_RHD1_y = avg(points[3*ref_RHD1_ids[0]+1], points[3*ref_RHD1_ids[1]+1]);
    int ref_RHD1_x_abs = avg(points[3*ref_RHD1_ids[0]], points[3*ref_RHD1_ids[1]]) * ms.width;

    int ref_RHD1_y_abs = ref_RHD1_y* ms.height;

    // std::cout<<"hair before "<<ref_RHD1_y_abs<<std::endl;
    while(hair_mask.at<uchar>(ref_RHD1_y_abs--,ref_RHD1_x_abs)* (1.0 / 255.0) < 0.5);
    // std::cout<<"hair after "<<ref_RHD1_y_abs<<std::endl;

    if(ref_RHD1_y_abs < 0)return;
    unit_size = abs(float(ref_RHD1_y_abs)/ms.height - ref_RHD1_y)/3.0f;
    ptr = points;
    



        //draw all 468 points
    /*data_num = 468;
    if(pdata_ == nullptr)pdata_ = new float [3 * data_num];
    for(int i=0;i<data_num;i++){
        pdata_[3*i] = ptr[3*i];//*2.0-1.0;
        pdata_[3*i+1] = ptr[3*i+1];//*2.0-1.0;
    }
    ptr = nullptr;
    */

    /*on_process(acu_ref_map);
    on_process(acu_map);
    ptr = nullptr;


    if(data_num == 0){
        if(draw_ref)for(auto p : acu_ref_map)data_num+= p.second.symmetry?2:1;
        if(draw_all_points){
            for(auto p : acu_map)data_num+= p.second.symmetry?2:1;
        }else{
            for(auto p : acu_map)
            if(p.second.channel == targe_ch) data_num+= p.second.symmetry?2:1;

        }
        std::cout<<"----total num-----"<<data_num<<std::endl;
        pdata_ = new float[3 * data_num];
    }
    int idx = 0;

    if(draw_ref) gen_mapped_points(acu_ref_map, idx);

    if(draw_acu_points) gen_mapped_points(acu_map, idx);*/

    gen_all_points(points, data_num);
        
    prenderer->Draw(pdata_, data_num);

}
/*return vec4 ranging [0,1] x increase to right, y increase down..IDK */
void acuGenerator::gen_all_points(const float* points, int& data_num){
    data_num = 468;
    pdata_ = new float[data_num*3];
    memcpy(pdata_, points, 3*data_num*sizeof(float));    
}

void acuGenerator::onDestroy(){
    
}