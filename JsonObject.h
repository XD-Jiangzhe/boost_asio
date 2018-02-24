#ifndef FND_JSONG_OBJECT_H
#define FND_JSONG_OBJECT_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//{"name" : "abc"}
//{"infomation" : "afddf"}
//{"name" : "abc" ,"infomation" : "afddf"}
#include <sstream>
#include <string>

using ptree = boost::property_tree::ptree;

inline std::string ptreeToJsonString(const ptree& tree)
{
    std::stringstream ss;
    boost::property_tree::write_json(ss, tree, false);
    return ss.str();
}



#endif