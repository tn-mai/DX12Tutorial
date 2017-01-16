/**
* @file TitleScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_
#define DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"

class TitleScene : public Scene::Scene
{
public:
	enum ExitCode
	{
		ExitCode_MainGame,
		ExitCode_Option,
	};

	static ::Scene::ScenePtr Create();

	virtual bool Load() override;
	virtual bool Unload() override;
	virtual int Update(double delta) override;
	virtual void Draw(Graphics::Graphics& graphics) const override;

private:
	TitleScene();
	TitleScene(const TitleScene&) = delete;
	TitleScene& operator=(const TitleScene&) = delete;

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

#endif // DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_