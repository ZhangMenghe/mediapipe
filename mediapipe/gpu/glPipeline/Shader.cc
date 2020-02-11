#include "mediapipe/gpu/glPipeline/Shader.h"
#include "mediapipe/framework/port/status.h"
#include <sstream>

using namespace mediapipe;
using namespace glm;
using namespace std;

Shader::~Shader() {
    for (const auto& p : mPrograms) {
        glDeleteProgram(p.second.mProgram);
        for (const auto& s : p.second.mShaders)
            glDeleteShader(s);
    }
}
void Shader::AddShader(GLenum type, const std::string content){
    mShadersToLink.emplace(type, content);
}
bool Shader::CompileAndLink(){
//    vector<vector<string>> keywords;
    available_keywords_.push_back(vector<string>());
    available_keywords_[0].push_back("");
    // create keyword variants
    for (const auto it : mShadersToLink) {
        istringstream iss(it.second);
        string line;
        while(getline(iss, line)){
            unsigned int kwc = available_keywords_.size();
            stringstream ss(line);
            string token;
            int mode = 0;
            while (getline(ss, token, ' ')) {
                if (mode == 2) {
                    // create variants
                    for (unsigned int i = 0; i < kwc; i++) {
                        vector<string> k(available_keywords_[i]);
                        int len = token.size();
                        if(token[len-1] == '\r') token = token.substr(0, len-1);
                        k.push_back(token);
                        available_keywords_.push_back(k);
                    }
                } else {
                    if (token == "#pragma")
                        mode = 1;
                    else if (mode == 1 && token == "multi_compile")
                        mode = 2;
                }
            }
        }
    }

//    LOGI("===Compiling %d shader variants\n", (int)keywords.size());

    for (auto& it : available_keywords_) {
        it.erase(it.begin());
        string kw = "";
        std::vector<std::string> kwv;
        for (const auto& k : it){kwv.push_back(k);kw += k + " ";}
        LOG(INFO)<<"===kw set:"<<kw;
        ShaderProgram cpr;
        if(!LinkShader(kwv, cpr))
            return false;
        mPrograms.emplace(kw, cpr);
    }
    active_keywords_ = vector<unordered_set<string>>(available_keywords_.size());
    return true;
}

GLuint Shader::Use(){
    string kw = "";
    for (auto& it : active_keywords_)
        for (const auto& k : it)
            kw += k + " ";
    // if(kw.size())
    //     LOGE("###### kw: %s", kw.c_str());
    assert(mPrograms.count(kw));

    GLuint p = mPrograms.at(kw).mProgram;
    glUseProgram(p);
    return p;
}

GLuint CompileShader(GLenum type, string content, vector<string> keywords) {
    //construct key_word line
    string key_word_str = "";
    if(!keywords.empty()){
//        key_word_str = "#define";
        for (const auto& kw : keywords){key_word_str+="#define " + kw + "\n";}
//        key_word_str += "\n";
//        LOGE("#####DEFINEl %s", key_word_str.c_str());
    }

    bool insertLine = false;
    string firstLine;
    istringstream iss(content);
    while(getline(iss, firstLine)){
        if (firstLine.substr(0, 8) == "#version"){
            content.insert(iss.tellg(), key_word_str);
            insertLine = true;
            break;
        }
    }
    if(!insertLine)
        content.insert(0, key_word_str);

    const char* src = content.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        vector<GLchar> info(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, info.data());

        string kw = "";
        for (const auto& it : keywords) kw += it + " ";
        glDeleteShader(shader);
        return 0;
    } else {
        return shader;
    }
}

bool Shader::LinkShader(std::vector<std::string> keywords, ShaderProgram& pgm){
    // compile shaders with keywords
//    LOGE("########keywords num :%d", keywords.size());
//    if(keywords.size() > 1)
//        LOGE("=====stop here2");
    for (const auto& it : mShadersToLink){
        GLuint shader_id = CompileShader(it.first, it.second, keywords);
        if(!shader_id)
            return false;
        pgm.mShaders.push_back(shader_id);
    }

    pgm.mProgram = glCreateProgram();
    for (const auto& s : pgm.mShaders)
        glAttachShader(pgm.mProgram, s);
    glLinkProgram(pgm.mProgram);

    GLint isLinked = 0;
    glGetProgramiv(pgm.mProgram, GL_LINK_STATUS, (int*)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(pgm.mProgram, GL_INFO_LOG_LENGTH, &maxLength);
        GLchar* info = (GLchar*)malloc(maxLength);
        glGetProgramInfoLog(pgm.mProgram, maxLength, &maxLength, info);
        LOG(ERROR)<<"===Could not link program:"<<info;
        free(info);
        glDeleteProgram(pgm.mProgram);
        for (const auto& s : pgm.mShaders)
            glDeleteShader(s);
    } else {
        for (const auto& s : pgm.mShaders)
            glDetachShader(pgm.mProgram, s);
    }
    return true;
}
void Shader::EnableKeyword(std::string keyword){
    int line_level = 0;
    for (auto& it : available_keywords_) {
        for (const auto& k : it){
            if(k == keyword){active_keywords_[line_level].emplace (keyword); return;}
        }
        line_level++;
    }
}
void Shader::DisableKeyword(std::string keyword){
    for(auto& it: active_keywords_){
        for (auto k = it.begin();k!=it.end();k++){
            if(*k == keyword){it.erase(k); return;}
        }
    }
}