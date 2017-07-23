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
#include <random>

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
	void UpdatePlayer(double delta);
	void UpdatePlayerShot(double delta);
	void UpdateEnemy(double delta);
	void UpdateEnemyShot(double delta);
	void GenerateEnemy(double delta);
	void CollisionPlayerShotAndEnemy();

	Resource::Texture texBackground;
	Resource::Texture texLogo;
	Resource::Texture texFont;
    std::vector<Sprite::Sprite> sprBackground;
	std::vector<Sprite::Sprite> sprLogo;
	std::vector<Sprite::Sprite> sprFont;
    Sprite::FilePtr cellFile;
    Sprite::CellList fontCellList;
	AnimationFile animationFile;
	double time;
	bool started;
	Audio::SoundPtr seStart;

	Audio::SoundPtr sePlayerShot;
	Audio::SoundPtr seBlast;
	Resource::Texture texCharacter;
	std::mt19937 rnd;
	std::vector<Sprite::Sprite> sprPlayer;
	std::vector<Sprite::Sprite*> sprPlayerShotFree;
	std::vector<Sprite::Sprite> sprEnemy;
	std::vector<Sprite::Sprite*> sprEnemyFree;
	std::vector<Sprite::Sprite*> sprEnemyShotFree;
	double entryTimer;
};

#endif // DX12TUTORIAL_SRC_SCENE_TITLESCENE_H_