// Simple Test
#include <iostream>

#include "json.h"

int main() {
	using namespace DianaJSON;
	Json json;
	std::string context{"{\"n\":123, \"s\":\"string\", \"arr\":[1.0, null, false, \"array\"], \"o\":{\"o1\":true, \"o2\":[2.0, true, \"array2\"], \"o3\": 2.0}}"};
	std::string errorText;
	json = Json::parse(context, errorText);

	//	Json::_object obj = json.toObject();
	//	for (const auto &item : obj) {
	//		std::cout << "Key: " << item.first;
	//		std::cout << " Value: ";
	//		if()
	//
	//		std::cout << std::endl;
	//	}
	std::cout << json << std::endl;

	auto jn = json["n"];
	std::cout << jn.toDouble() << std::endl;

	auto js = json["s"];
	if (js.getType() == JsonValueType::String)
		std::cout << js.toString() << std::endl;

	auto ja = json["arr"];
	if (ja.getType() == JsonValueType::Array) {
		auto array = ja.toArray();
		for (auto item : array) {
			if (item.isNumber())
				std::cout << item.toDouble() << " ";
			if (item.isNull())
				std::cout << "null"
						  << " ";
			if (item.isBoolean())
				std::cout << std::boolalpha << item.toBool() << " ";
			if (item.isString())
				std::cout << item.toString() << " ";
		}
		std::cout << std::endl;
	}

	auto jo = json["o"];
	if (jo.getType() == JsonValueType::Object) {
		auto obj = jo.toObject();
		for (auto keyValue : obj) {
			std::cout << "Key: " << keyValue.first << " Value: ";
			if (keyValue.second.isNumber())
				std::cout << keyValue.second.toDouble() << " ";
			if (keyValue.second.isNull())
				std::cout << "null"
						  << " ";
			if (keyValue.second.isBoolean())
				std::cout << std::boolalpha << keyValue.second.toBool() << " ";
			if (keyValue.second.isString())
				std::cout << keyValue.second.toString() << " ";
			if (keyValue.second.isArray()) {
				auto array = keyValue.second.toArray();
				for (auto item : array) {
					if (item.isNumber())
						std::cout << item.toDouble() << " ";
					if (item.isNull())
						std::cout << "null"
								  << " ";
					if (item.isBoolean())
						std::cout << std::boolalpha << item.toBool() << " ";
					if (item.isString())
						std::cout << item.toString() << " ";
				}
			}
			std::cout << std::endl;
		}
	}

	return 0;
}