/**
* @file EndingScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_ENDINGSCENE_H_
#define DX12TUTORIAL_SRC_SCENE_ENDINGSCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"

class EndingScene : public Scene::Scene
{
public:
	static ::Scene::ScenePtr Create();

	virtual bool Load(::Scene::Context&) override;
	virtual bool Unload(::Scene::Context&) override;
	virtual int Update(::Scene::Context&, double delta) override;
	virtual void Draw(Graphics::Graphics& graphics) const override;

private:
	EndingScene();
	EndingScene(const EndingScene&) = delete;
	EndingScene& operator=(const EndingScene&) = delete;

	Resource::Texture texBackground;
	Resource::Texture texLogo;
	Resource::Texture texFont;
	std::vector<Sprite::Sprite> sprBackground;
	std::vector<Sprite::Sprite> sprLogo;
	std::vector<Sprite::Sprite> sprFont;
	Sprite::FilePtr cellFile;
	AnimationFile animationFile;
	double time;
};

#endif // DX12TUTORIAL_SRC_SCENE_ENDINGSCENE_H_
