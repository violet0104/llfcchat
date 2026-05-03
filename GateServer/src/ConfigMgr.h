#pragma once
#include "const.h"

struct SectionInfo {
    // 编译器自动生成
    // SectionInfo(){}
    // ~SectionInfo(){_section_datas.clear();}

    // SectionInfo(const SectionInfo& src) {
    //     _section_datas = src._section_datas;
    // }
    
    // SectionInfo& operator=(const SectionInfo& src) {
    //     if (&src == this) {
    //         return *this;
    //     }
    //     this->_section_datas = src._section_datas;
    // }

    // 重载SectionInfo的[]
    std::map<std::string, std::string> _section_datas;
    const std::string& operator[](const std::string& key) {
        return _section_datas[key];
    }
};


class ConfigMgr
{
public:
    ~ConfigMgr() { _config_map.clear(); }
    ConfigMgr(const ConfigMgr&) = delete;
    ConfigMgr& operator=(const ConfigMgr&) = delete;

    // 重载ConfigMgr的[]
    SectionInfo& operator[](const std::string& section) {
        // 没有section，则自动插入默认值
        return _config_map[section];
    }

    // C++11以后，多线程访问是线程安全的
    static ConfigMgr& Inst() {
        // 静态局部变量：程序生命周期内唯一，仅初始化1次
        // 静态局部变量的初始化是线程安全的
        static ConfigMgr cfg_mgr;
        return cfg_mgr;
    }

private:
    ConfigMgr();
    // 存储section和key-value对的map  
    std::map<std::string, SectionInfo> _config_map;
};
