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

	static ::Scene::ScenePtr Creat();

	virtual bool Load();
	virtual bool Unload();
	virtual int Update(double delta);
	virtual void Draw(::Scene::Graphics& graphics) const;

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
	AnimationFile animationFile;
	double time;
};

#endif // DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_