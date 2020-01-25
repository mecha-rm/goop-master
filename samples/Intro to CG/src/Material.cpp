#include "Material.h"

#include "imgui.h"
#include <GLM/gtc/type_ptr.hpp>

void Material::Apply() {
	for (auto& kvp : myMat4s)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);
	for (auto& kvp : myVec4s)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);
	for (auto& kvp : myVec3s)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);
	for (auto& kvp : myVec2s)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);
	for (auto& kvp : myFloats)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);
	for (auto& kvp : myInts)
		myShader->SetUniform(kvp.first.c_str(), kvp.second);

	if (IsBlendingEnabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}

	if (IsCullingEnabled) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
		
	// New in tutorial 07
	int slot = 0;
	for (auto& kvp : myTextures) {
		if (kvp.second.Sampler != nullptr)
			kvp.second.Sampler->Bind(slot);
		else
			TextureSampler::Unbind(slot);
		
		kvp.second.Texture->Bind(slot);
		myShader->SetUniform(kvp.first.c_str(), slot);
		slot++;
	}
}

Material::Sptr Material::Clone()
{
	Sptr result = std::make_shared<Material>(*this);

	return result;
}

void Material::DrawEditor(const std::vector<TextureSampler::Sptr>& samplerOptions, const std::vector<Texture2D::Sptr>& textureOptions) {
	for (auto& kvp : myMat4s) {
		ImGui::LabelText("%s: (not implemented)", kvp.first.c_str());
	}
	for (auto& kvp : myVec4s) {
		ImGui::DragFloat4(kvp.first.c_str(), glm::value_ptr(kvp.second), 0.01f);
	}
	for (auto& kvp : myVec3s) {
		ImGui::DragFloat3(kvp.first.c_str(), glm::value_ptr(kvp.second), 0.01f);
	}
	for (auto& kvp : myVec2s) {
		ImGui::DragFloat2(kvp.first.c_str(), glm::value_ptr(kvp.second), 0.01f);
	}
	for (auto& kvp : myFloats) {
		ImGui::DragFloat(kvp.first.c_str(), &kvp.second, 0.01f);
	}
	for (auto& kvp : myInts) {
		ImGui::DragInt(kvp.first.c_str(), &kvp.second, 0.01f);
	}

	// New in tutorial 07
	int slot = 0;
	for (auto& kvp : myTextures) {
		ImGui::PushID(kvp.first.c_str());
		if (ImGui::BeginCombo("Sampler", kvp.second.Sampler ? kvp.second.Sampler->GetDebugName().c_str() : "<none>")) {
			for (auto& sampler : samplerOptions) {
				if (ImGui::Selectable(sampler ? sampler->GetDebugName().c_str() : "<none>", sampler == kvp.second.Sampler)) {
					kvp.second.Sampler = sampler;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("Texture", kvp.second.Texture ? kvp.second.Texture->GetDebugName().c_str() : "<none>")) {
			for (auto& texture : textureOptions) {
				if (ImGui::Selectable(texture->GetDebugName().c_str(), texture == kvp.second.Texture)) {
					kvp.second.Texture = texture;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
}
