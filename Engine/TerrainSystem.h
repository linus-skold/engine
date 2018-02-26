#pragma once
#include "QuadTree.h"
#include "engine_shared.h"
class Terrain;

namespace test
{

	struct Position
	{
		float x, y;
	};

	struct AABB
	{
		Position m_Pos;
		float m_Halfwidth;

		bool Intersect(Position position);
		bool Intersect(Position position, float radius);


	};

	struct Leaf
	{
		Leaf();
		~Leaf();

		AABB m_AABB;
		Leaf* m_Parent = nullptr;
		Leaf* m_Children[4];
		int m_Size = 0;
		int m_Depth = 0;
		int m_Index = 0;
		bool Insert(Position pos);
		void subdivide();

		bool Render();

		void Reset();
		Terrain* m_Terrain = nullptr;


		bool isNeighbour(test::Leaf* leaf);

	};



	class QuadTree
	{
	public:
		QuadTree() = default;

		void Init(Position xy);
		void Insert(Position xy);
		void Draw();


		void Update(float x, float y);

	private:
		Leaf* m_Root = nullptr;

	};


}





class TerrainSystem
{
public:
	TerrainSystem();

	void Update();

	float m_X = 0.f;
	float m_Y = 0.f;

private:
	test::QuadTree m_Tree;

};
