/**
* @file TitleScene.h
*/
#ifndef DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_
#define DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_
#include "../Scene.h"
#include "../Texture.h"
#include "../Sprite.h"
#include "../Animation.h"
#include "../Audio.h"

class TitleScene : public Scene::Scene
{
public:
	enum ExitCode
	{
		ExitCode_MainGame,
		ExitCode_Option,
	};

	static ::Scene::ScenePtr Create();

	virtual bool Load(::Scene::Context&) override;
	virtual bool Unload(::Scene::Context&) override;
	virtual int Update(::Scene::Context&, double delta) override;
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
	bool started;
	Audio::SoundPtr seStart;
};

#endif // DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_