#include "TranslationComponent.h"
#include <CommonLib/Math/Vector/Vector.h>
void TranslationComponent::Deserialize(const rapidjson::Value& value)
{
	auto& position = value["position"].GetArray();
	myOrientation.SetPosition({ (float)position[0].GetDouble(), (float)position[1].GetDouble(), (float)position[2].GetDouble(), 1.f });

	auto& up = value["up"].GetArray();

	const float up_x = (float)up[0].GetDouble();
	const float up_y = (float)up[1].GetDouble();
	const float up_z = (float)up[2].GetDouble();

	auto& fwd = value["forward"].GetArray();

	const float fwd_x = (float)fwd[0].GetDouble();
	const float fwd_y = (float)fwd[1].GetDouble();
	const float fwd_z = (float)fwd[2].GetDouble();

	CU::Vector4f vec_up = { up_x, up_y, up_z, 0 };
	CU::Vector4f vec_fwd = { fwd_x, fwd_y, fwd_z, 0 };

	CU::Vector4f right = CU::Math::Cross(vec_up, vec_fwd);
	right.w = 0;
	
	myOrientation.SetUp(vec_up);
	myOrientation.SetForward(vec_fwd);
	myOrientation.SetRight(right);


	auto& scale = value["scale"].GetArray();

	const float scale_x = (float)scale[0].GetDouble();
	const float scale_y = (float)scale[1].GetDouble();
	const float scale_z = (float)scale[2].GetDouble();

	myOrientation = CU::Matrix44f::CreateScaleMatrix(scale_x, scale_y, scale_z, 1) * myOrientation;



}
