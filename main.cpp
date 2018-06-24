#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>

#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <random>
#include <ctime>

sf::Vector2f normalize(const sf::Vector2f& source)
{
	float length = sqrt((source.x * source.x) + (source.y * source.y));
	if (length != 0)
		return sf::Vector2f(source.x / length, source.y / length);
	else
		return source;
}

float deg2rad(float deg)
{
	return deg * 0.0174533f;
}

sf::Vector2f rotateVec2f(sf::Vector2f v, float deg)
{
	sf::Vector2f out;

	float theta = deg2rad(deg);

	float cs = cos(theta);
	float sn = sin(theta);

	out.x = v.x * cs - v.y * sn;
	out.y = v.x * sn + v.y * cs;

	return out;
}

int randInt(int min, int max, int seed)
{
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(min, max);

	return distribution(generator);
}

float randFloat(float min, float max, int seed)
{
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> distribution(min, max);

	return distribution(generator);
}

int clampInt(int value, int min, int max)
{
	int out = value;

	if (value < min)
		out = min;

	if (value > max)
		out = max;

	return out;
}

float clampFloat(float value, float min, float max)
{
	float out = value;

	if (value < min)
		out = min;

	if (value > max)
		out = max;

	return out;
}

struct GenVars
{
	float thickness_start = 10;
	float thickness_drop_min = .75;
	float thickness_drop_max = .9;

	float length_start = 125;
	float length_drop_min = .75;
	float length_drop_max = .9;

	float rotation_start = 0;
	float rotation_min = -10;
	float rotation_max = 10;

	float split_chance = 1;
	int split_amount = 2;

	int segments = 13;

	unsigned int seed = 0;



	sf::Vector2f pos = sf::Vector2f(400, 600);
	sf::Vector2f dir = sf::Vector2f(0, -1);

	float thickness = thickness_start;
	float length = length_start;
	float rotation = rotation_start;
};

struct segment
{
	sf::Vector2f p0;
	sf::Vector2f p1;

	float thickness0 = 10;
	float thickness1 = 10;

	sf::VertexArray shape;

	sf::Color color = sf::Color::White;

	GenVars gv;

	std::vector<segment> children;

	void Generate()
	{
		p0 = gv.pos;
		thickness0 = gv.thickness;

		gv.rotation = randInt(gv.rotation_min, gv.rotation_max, gv.seed++);
		gv.dir = rotateVec2f(gv.dir, gv.rotation);
		gv.thickness = gv.thickness * randFloat(gv.thickness_drop_min, gv.thickness_drop_max, gv.seed++);
		gv.length = gv.length * randFloat(gv.length_drop_min, gv.length_drop_max, gv.seed++);
		gv.pos = gv.pos + (gv.dir * gv.length);

		p1 = gv.pos;
		thickness1 = gv.thickness;

		GenerateMesh();

		gv.segments--;

		if (gv.segments <= 0)
			return;

		float split_test = randFloat(0.0f, 1.0f, gv.seed++);

		if (split_test < gv.split_chance)
		{
			GenVars gvn = gv;
			gvn.rotation += gv.rotation + gv.rotation_min;
			gvn.dir = rotateVec2f(gvn.dir, gvn.rotation);
			gvn.seed++;

			segment sn;
			sn.gv = gvn;

			children.push_back(sn);

			gvn.rotation += gv.rotation + gv.rotation_max;
			gvn.dir = rotateVec2f(gvn.dir, gvn.rotation);
			gvn.seed++;

			sn.gv = gvn;

			children.push_back(sn);

			for (int i = 0; i < children.size(); i++)
			{
				children[i].Generate();
			}
		}
		else
		{			
			GenVars gvn = gv;
			gvn.seed++;

			segment sn;
			sn.gv = gvn;

			children.push_back(sn);

			children[0].Generate();
		}
	}

	void GenerateMesh()
	{
		shape = sf::VertexArray(sf::TriangleStrip, 4);

		// Calculate direction from p0 to p1
		sf::Vector2f dir = p1 - p0;
		dir = normalize(dir);

		// rotate dir 90
		dir = rotateVec2f(dir, 90.0f);

		shape[0].position = p0 + dir * (thickness0 / 2);
		shape[1].position = p0 - dir * (thickness0 / 2);

		shape[2].position = p1 + dir * (thickness1 / 2);
		shape[3].position = p1 - dir * (thickness1 / 2);


		for (int i = 0; i < 4; i++)
			shape[i].color = color;
	}

	void Draw(sf::RenderWindow& window)
	{
		for (int i = 0; i < children.size(); i++)
			children[i].Draw(window);

		window.draw(shape);
	}

	void Clear()
	{
		children.clear();
	}
};

int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");

	int seed = time(NULL);
	std::cout << "seed: " << seed << std::endl;
	
	segment sbase;
	sbase.gv.seed = seed;

	segment s = sbase;
	s.gv.seed = seed;
	s.Generate();


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			s.Clear();
			seed = randInt(0, 10000000, seed);
			sbase.gv.seed = seed;
			std::cout << "seed: " << seed << std::endl;
			s = sbase;			
			s.Generate();
		}

		window.clear();

		s.Draw(window);

		window.display();
	}

	return 0;
}