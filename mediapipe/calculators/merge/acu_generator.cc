#include "acu_generator.h"
#include <iostream>
#include <fstream>
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
        r1 = (func_name == "GetX")? ptr[2*rid]:ptr[2*rid+1];
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
    std::string filename = "mediapipe/res/acu.csv";
    std::ifstream myFile(filename);
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");
    std::string line, colname;
    std::string val;

    // Read the column names
    if(!myFile.good()){std::cerr<<"Failed to open file";return;}
    
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
void acuGenerator:: onDraw(faceRect rect, cv::Mat hair_mask, const float* points){
    //get unit size
    auto ms = hair_mask.size();
    
    float ry = (points[17] + points[19]) * 0.5f;
    int y = int((1.0-(ry+0.5)) * ms.height);
    int x = ((points[16] + points[18])*0.5 + 0.5) * ms.width;
    while(hair_mask.at<uchar>(y--,x) == 0);

    if(y == 0)return;

    unit_size = (((1.0 - float(y) / ms.height) - 0.5) - ry) /3;
    std::cout<<"cal unit size: "<<unit_size<<std::endl;

    ptr = points;
    
    on_process(acu_ref_map);
    on_process(acu_map);
    ptr = nullptr;
}
void acuGenerator::onDestroy(){
    
}