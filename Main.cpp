#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include<SFML\Graphics.hpp>
#include <string>
#include <vector>

#define PI 3.14159265359

unsigned int buffer_width = 256, buffer_height = 144;
char screen_buffer[144][256];

struct Vertex
{
	float x, y, z;
	Vertex(){}
	Vertex(float dx, float dy, float dz)
		: x(dx), y(dy), z(dz) {}

	void scale(float multiplier)
	{
		x *= multiplier;
		y *= multiplier;
		z *= multiplier;
	}

	void rotateZ(float ang)
	{
		float angle = ang * (PI / 180);

		float dx = ((float)x * cosf(angle)) - ((float)y * sinf(angle));
		float dy = ((float)x * sinf(angle)) + ((float)y * cosf(angle));

		x = dx;
		y = dy;
	}

	void rotateY(float ang)
	{
		float angle = ang * (PI / 180);

		float dx = ((float)z * sinf(angle)) + ((float)x * cosf(angle));
		float dz = ((float)z * cosf(angle)) - ((float)x * sinf(angle));

		x = dx;
		z = dz;
	}

	void translate(float dx, float dy, float dz)
	{
		x += dx;
		y += dy;
		z += dz;
	}

};

float AR = (float)buffer_height / (float)buffer_width;
float FOV = 1 / tanf((90 * (PI / 180)) / 2);
float near = 1, far = 100;

float projection[4][4] =
{
{AR * FOV, 0,0,0},
{0,FOV,0,0},
{0,0,far / (far - near),1},
{0,0,(-far * near) / (far - near),0},
};

Vertex VectorMultiplyMatrix(const Vertex& v, const float(&list)[4][4])
{
	Vertex nv;
	nv.x = v.x * list[0][0] + v.y * list[1][0] + v.z * list[2][0] + list[3][0];
	nv.y = v.x * list[0][1] + v.y * list[1][1] + v.z * list[2][1] + list[3][1];
	nv.z = v.x * list[0][2] + v.y * list[1][2] + v.z * list[2][2] + list[3][2];

	float w = v.x * list[0][3] + v.y * list[1][3] + v.z * list[2][3] + list[3][3];

	if (w != 0.0f)
	{
		nv.x /= w;
		nv.y /= w;
		nv.z /= w;
	}

	return Vertex(nv.x, nv.y, nv.z);
};

float view[4][4] =
{
{0,0,0,0},
{0,0,0,0},
{0,0,0,0},
{0,0,0,0},
};


struct Polygon
{
	Vertex a, b, c;
	Polygon(){}
	Polygon(const Vertex da, const Vertex db, const Vertex dc)
		: a(da), b(db), c(dc){}

	void scale(float multiplier)
	{
		a.scale(multiplier);
		b.scale(multiplier);
		c.scale(multiplier);
	}

	void translate(float x, float y, float z)
	{
		a.translate(x,y,z);
		b.translate(x,y,z);
		c.translate(x,y,z);
	}

	void rotateZ(float angle)
	{
		a.rotateZ(angle);
		b.rotateZ(angle);
		c.rotateZ(angle);
	}

	void rotateY(float angle)
	{
		a.rotateY(angle);
		b.rotateY(angle);
		c.rotateY(angle);
	}

	void project()
	{
		a = VectorMultiplyMatrix(a, projection);
		b = VectorMultiplyMatrix(b, projection);
		c = VectorMultiplyMatrix(c, projection);
	}

	void perspective()
	{
		a = VectorMultiplyMatrix(a, view);
		b = VectorMultiplyMatrix(b, view);
		c = VectorMultiplyMatrix(c, view);
	}
};

float findY(float ax, float ay, float bx, float by, float x)
{
	float slope = ((float)ay - (float)by) / ((float)ax - (float)bx);
	float intercept = (float)ay - (slope * (float)ax);
	return slope * x + intercept;
}

void drawLine(float ax, float ay, float bx, float by)
{
	if (bx > ax)
	{
		for (size_t i = ax; i < bx; i++)
		{
			if(i >= 0 && i <= buffer_width && (int)findY(ax, ay, bx, by, i) >= 0 && (int)findY(ax, ay, bx, by, i) <= buffer_height)
			screen_buffer[(int)findY(ax, ay, bx, by, i)][i] = 'Z';
		}
	}
	else
	{
		for (size_t i = bx; i < ax; i++)
		{
			if (i >= 0 && i <= buffer_width && (int)findY(ax, ay, bx, by, i) >= 0 && (int)findY(ax, ay, bx, by, i) <= buffer_height)
			screen_buffer[(int)findY(ax, ay, bx, by, i)][i] = 'Z';
		}
	}
}
void drawLine(const Vertex& a, const Vertex& b)
{
	if (b.x > a.x)
	{
		for (size_t i = a.x; i < b.x; i++)
		{
			if (i >= 0 && i <= buffer_width && (int)findY(a.x, a.y, b.x, b.y, i) >= 0 && (int)findY(a.x, a.y, b.x, b.y, i) <= buffer_height)
				screen_buffer[(int)findY(a.x, a.y, b.x, b.y, i)][i] = 'Z';
		}
	}
	else
	{
		for (size_t i = b.x; i < a.x; i++)
		{
			if (i >= 0 && i <= buffer_width && (int)findY(a.x, a.y, b.x, b.y, i) >= 0 && (int)findY(a.x, a.y, b.x, b.y, i) <= buffer_height)
				screen_buffer[(int)findY(a.x, a.y, b.x, b.y, i)][i] = 'Z';
		}
	}
}
void drawPolygon(const Polygon& triangle)
{
	drawLine(triangle.a, triangle.b);
	drawLine(triangle.b, triangle.c);
	drawLine(triangle.c, triangle.a);
}

struct Cube
{
	Polygon* triangles[12];
	Cube()
	{
		triangles[0] = new Polygon(Vertex(-1, -1, 1), Vertex(-1, 1, 1), Vertex(1, 1, 1));
		triangles[1] = new Polygon(Vertex(-1, -1, 1), Vertex(1, -1, 1), Vertex(1, 1, 1));

		triangles[2] = new Polygon(Vertex(-1, -1, -1), Vertex(-1, 1, -1), Vertex(1, 1, -1));
		triangles[3] = new Polygon(Vertex(-1, -1, -1), Vertex(1, -1, -1), Vertex(1, 1, -1));

		triangles[4] = new Polygon(Vertex(-1, -1, -1), Vertex(-1, 1, -1), Vertex(-1, 1, 1));
		triangles[5] = new Polygon(Vertex(-1, -1, -1), Vertex(-1, -1, 1), Vertex(-1, 1, 1));

		triangles[6] = new Polygon(Vertex(1, -1, -1), Vertex(1, 1, -1), Vertex(1, 1, 1));
		triangles[7] = new Polygon(Vertex(1, -1, -1), Vertex(1, -1, 1), Vertex(1, 1, 1));

		triangles[8] = new Polygon(Vertex(-1, -1, -1), Vertex(1, -1, -1), Vertex(1, -1, 1));
		triangles[9] = new Polygon(Vertex(-1, -1, -1), Vertex(-1, -1, 1), Vertex(1, -1, 1));

		triangles[10] = new Polygon(Vertex(-1, 1, -1), Vertex(1, 1, -1), Vertex(1, 1, 1));
		triangles[11] = new Polygon(Vertex(-1, 1, -1), Vertex(-1, 1, 1), Vertex(1, 1, 1));
	}

	void scale(float multiplier)
	{
		for (auto i : triangles)
		{
			i->scale(multiplier);
		}
	}

	void rotateZ(float angle)
	{
		for (auto i : triangles)
		{
			i->rotateZ(angle);
		}
	}

	void rotateY(float angle)
	{
		for (auto i : triangles)
		{
			i->rotateY(angle);
		}
	}

	void translate(int x, int y, int z)
	{
		for (auto i : triangles)
		{
			i->translate(x,y,z);
		}
	}

	void project()
	{
		for (auto i : triangles)
		{
			i->project();
		}
	}

	void perspective()
	{
		for (auto i : triangles)
		{
			i->perspective();
		}
	}

	void draw()
	{
		for (auto i : triangles)
		{
			drawPolygon(*i);
		}
	}

	~Cube()
	{
		for (auto i : triangles)
		{
			delete i;
		}
	}
};

Vertex cross(const Vertex& a, const Vertex& b)
{
	return Vertex(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

void lookAt(const Vertex& from, const Vertex& to, const Vertex& temp = {0,1,0})
{
	Vertex forward(from.x - to.x, from.y - to.y, from.z - to.z);
	float forwMag = sqrt(from.x * from.x + from.y * from.y + from.z * from.z);
	forward.x /= forwMag; forward.y /= forwMag; forward.z /= forwMag;

	Vertex right = temp;
	float rightMag = sqrt(temp.x * temp.x + temp.y * temp.y + temp.z * temp.z);
	right.x /= rightMag; right.y /= rightMag; right.z /= rightMag;

	right = cross(right, forward);

	Vertex up = cross(forward, right);

	view[0][0] = right.x; view[0][1] = right.y; view[0][2] = right.z;
	view[1][0] = up.x; view[1][1] = up.y; view[1][2] = up.z;
	view[2][0] = forward.x; view[2][1] = forward.y; view[2][2] = forward.z;
	view[3][0] = from.x; view[3][1] = from.y; view[3][2] = from.z;

}

void floodFill(unsigned int x, unsigned int y)
{
	printf("X: %i  Y: %i \n", x, y);

	if (screen_buffer[y][x] == 'M')
	{
		screen_buffer[y][x] = 'Z';
		floodFill(x - 1, y);
		floodFill(x + 1, y);
		floodFill(x, y + 1);
		floodFill(x, y - 1);
	}
}

int main()
{
	sf::Text text;
	sf::Font font;
	font.loadFromFile("C:/Users/tlins/Documents/Visual Studio 2017/Projects/ASCII game/arial.ttf");
	text.setFont(font);
	text.setCharacterSize(3.75f);
	text.setFillColor(sf::Color::White);
	text.setPosition(0, 0);
	float width = 768, height = 433;

	sf::RenderWindow window(sf::VideoMode(width, height), "ASCII Project");
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(60);

	float ang = 0;
	float yvar = 20;

	Vertex cubePos(127, 72, 5);
	Vertex playerPos(127, 72, 5);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
			{
				window.close();
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			cubePos.z -= 1.0f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			cubePos.z += 1.0f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		{
			cubePos.x += 5.0f * AR;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			cubePos.x -= 5.0f * AR;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
		{
			cubePos.y -= 5.0f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
		{
			cubePos.y += 5.0f;
		}

		window.clear(sf::Color(0,0,0,1));

		//Update screen buffer
		for (size_t i = 0; i < buffer_height; i++)
		{
			for (size_t j = 0; j < buffer_width; j++)
			{
				screen_buffer[i][j] = 'M';
			}
		}

		lookAt(playerPos, Vertex(playerPos.x, playerPos.y, playerPos.z + 1.0f));

		Cube mesh;
		mesh.rotateZ(ang - 20);
		mesh.rotateY(ang);
		mesh.translate(0,0,cubePos.z);
		mesh.project();
		mesh.scale(80);
		mesh.translate(cubePos.x, cubePos.y,0);
		//mesh.perspective();
		mesh.draw();

		ang += 10;

		std::string temp = "";
		for (size_t i = 0; i < buffer_height; i++)
		{
			for (size_t j = 0; j < buffer_width; j++)
			{
				temp.push_back(screen_buffer[i][j]);
			}
			temp.append("\n");
		}
		text.setString(temp);


		window.draw(text);
		window.display();
	}

	return 0;
}