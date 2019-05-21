#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>

constexpr int cellLength = 40;
constexpr int lineWidth = 1;
constexpr int numColumns = 10;
constexpr int numRows = 20;
constexpr int padding = 80;
constexpr int windowX = 2*padding + numColumns*cellLength;
constexpr int windowY = 2*padding + numRows*cellLength;

//Structure representing a (x, y) coordinate pair/
struct position
{
	int x, y;
};

//Cells used to compose a cell map making up the game board
//Can be filled or unfilled with an arbitrary color
class cell
{
	private:
		position p;
		bool isFilled = false;
		sf::RectangleShape lines[4];
		sf::RectangleShape fill;

	public:
		cell(int x, int y);
		void configLines();
		void configFill(sf::Color fillColor);
		void setIsFilled(bool set);
		bool returnIsFilled();
		sf::RectangleShape returnLine(int d);
		sf::RectangleShape returnFill();
};

//Consturctor initializing the cells position in the cell map and calling the function that configures the cell's borders
cell::cell(int x, int y)
{
	p.x = x;
	p.y = y;
	configLines();
}

//Configures the cell's borders into SFML rectangles
void cell::configLines()
{
	for (int i = 0; i < 4; i++)
	{
		lines[i].setSize(sf::Vector2f(cellLength, lineWidth));
		lines[i].setFillColor(sf::Color::White);
		if (i == 0 || i == 1)
		{
			lines[i].setPosition((p.x * cellLength + padding), (p.y * cellLength + padding));
		}
		if (i == 1 || i == 3)
		{
			lines[i].setRotation(90.f);
		}
	}

	lines[2].setPosition((p.x * cellLength + padding), ((p.y + 1) * cellLength + padding));

	lines[3].setPosition(((p.x + 1) * cellLength + padding), (p.y * cellLength + padding));
}

//Configures the cell's fill into a SFML rectangle of arbitrary length
void cell::configFill (sf::Color fillColor)
{
	fill.setSize(sf::Vector2f(cellLength, cellLength));
	fill.setFillColor(fillColor);
	fill.setPosition((p.x * cellLength + padding), (p.y * cellLength + padding));
}

//Returns the SFML rectangle of the cell's line at direction d
//0 is up, 1 is left, 2 is down, 3 is right
sf::RectangleShape cell::returnLine(int d)
{
	return lines[d];
}

//Returns the SFML rectanlge of the cell's fill
sf::RectangleShape cell::returnFill()
{
	return fill;
}

//Sets whether or not the fill should be displayed
void cell::setIsFilled(bool set)
{
	isFilled = set;
}

//Returns whether or not the fill should eb displayed
bool cell::returnIsFilled()
{
	return isFilled;
}

//Blocks that represent each filled cell on the map
class block
{
	private:
		position p;
		//Default color of white
		sf::Color blockColor = sf::Color::White;
	public:
		void move(int dir);
		//Set position must be called
		void setPosition(int x, int y);
		position returnPosition();
		void setColor(sf::Color color);
		sf::Color returnColor();
};

//Function to move a block in an arbitrary direction
//0 is up, 1 is left, 2 is down, 3 is right
void block::move(int dir)
{
	if (dir == 0)
	{
		p.y--;
	}
	else if (dir == 1)
	{
		p.x--;
	}
	else if (dir == 2)
	{
		p.y++;
	}
	else if (dir == 3)
	{
		p.x++;
	}
}

//Sets a blocks position to an arbitrary point
void block::setPosition(int x, int y)
{
	p.x = x;
	p.y = y;
}

//Returns a block's position object
position block::returnPosition()
{
	return p;
}

//Sets a blocks color to an arbitrary value
void block::setColor(sf::Color color)
{
	blockColor = color;
}

//Returns a block's color
sf::Color block::returnColor()
{
	return blockColor;
}

//Tetrominos are groups of blocks
class tetromino
{
	private:
		// Block list making up ithe tetromino
		std::vector<block> blockList;
		position p;
		/* Shape IDs
		0: I
		1: O
		2: T
		3: J
		4: L
		5: S
		6: Z */
		int shape;
		/* Rotation values
		0: 0 degrees
		1: 90 degrees
		2: 180 degrees
		3: 270 degrees */
		int rotation;

	public:
		tetromino(int setShape);
		
		/* Move directions
		0: Up
		1: Left
		2: Down
		3: Right */
		void move(int direction);
		position returnPosition();
		/* Rotate directions
		-1: Counter-clockwise
		1: Clockwise */
		void rotate(int direction);
		
		std::vector<block> decompose(std::vector<block> setBlockList);

		void configBlockList();
		std::vector<block> returnBlockList();
};

//Constructor setting the shape ID, the x and y coordinates, and the default blockList values
tetromino::tetromino(int setShape)
{
	shape = setShape;
	p.x = numColumns/2;
	p.y = 0;
	rotation = 0;
	configBlockList();
}

//Move a whole tetromino in an arbitrary direction
//0: Up, 1: Left, 2: Down, 3: Right
void tetromino::move(int direction)
{
	try
	{
		if (direction < 0 || direction > 3)
		{
			throw std::runtime_error("Move direction out of bounds");
		}

		if (direction == 0)
		{
			p.y--;
		}
		else if (direction == 1)
		{
			p.x--;
		}
		else if (direction == 2)
		{
			p.y++;
		}
		else
		{
			p.x++;
		}
	}
	catch(std::exception const &e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

//Return the position of the tetromino
position tetromino::returnPosition()
{
	return p;
}
//Rotate a whole tetromino in an arbitrary direction
//-1: Counter-clockwise, 1: Clockwise
void tetromino::rotate(int direction)
{
	try
	{
		if (direction != -1 && direction != 1)
		{
			throw std::runtime_error("Rotation direction out of bounds");
		}

		rotation += direction;
		rotation %= 4;
	}
	catch(std::exception const &e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

//Returns an augmented block list where the give tetromino has decomposed to the blocks that make it up
std::vector<block> tetromino::decompose(std::vector<block> setBlockList)
{
	for (int i = 0; i < blockList.size(); i++)
	{
		block tempBlock = blockList[i];
		tempBlock.setPosition(blockList[i].returnPosition().x + p.x, blockList[i].returnPosition().y + p.y);
		setBlockList.push_back(tempBlock);
	}

	return setBlockList;
}

//Configures the whole block list of this tetromino
void tetromino::configBlockList()
{
	/* Tetromino Shapes:
	0 - I
	1 - O
	2 - T
	3 - J
	4 - L
	5 - S
	6 - Z */

	/* Rotation IDs:
	0 - 0   degrees
	1 - 90  degrees
	2 - 180 degrees
	3 - 270 degrees */

	//Four blocks for each tetromino
	block blocks[4];

	//I
	if (shape == 0)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 2,  0);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 0,  1);
			blocks[3].setPosition( 0,  2);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition(-2,  0);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition( 0, -2);
			blocks[1].setPosition( 0, -1);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 0,  1);
		}

		blocks[0].setColor(sf::Color::Cyan);
		blocks[1].setColor(sf::Color::Cyan);
		blocks[2].setColor(sf::Color::Cyan);
		blocks[3].setColor(sf::Color::Cyan);
	}
	//O
	else if (shape == 1)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 0,  0);
			blocks[1].setPosition( 1,  0);
			blocks[2].setPosition( 0,  1);
			blocks[3].setPosition( 1,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition(-1,  1);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition(-1, -1);
			blocks[1].setPosition( 0, -1);
			blocks[2].setPosition(-1,  0);
			blocks[3].setPosition( 0,  0);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 1, -1);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}

		blocks[0].setColor(sf::Color::Yellow);
		blocks[1].setColor(sf::Color::Yellow);
		blocks[2].setColor(sf::Color::Yellow);
		blocks[3].setColor(sf::Color::Yellow);
	}
	//T
	else if (shape == 2)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition(-1,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}

		blocks[0].setColor(sf::Color::Magenta);
		blocks[1].setColor(sf::Color::Magenta);
		blocks[2].setColor(sf::Color::Magenta);
		blocks[3].setColor(sf::Color::Magenta);
	}
	//J
	else if (shape == 3)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 0,  1);
			blocks[3].setPosition(-1,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition(-1, -1);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 1, -1);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 1,  1);
		}

		blocks[0].setColor(sf::Color::Blue);
		blocks[1].setColor(sf::Color::Blue);
		blocks[2].setColor(sf::Color::Blue);
		blocks[3].setColor(sf::Color::Blue);
	}
	//L
	else if (shape == 4)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 0,  1);
			blocks[3].setPosition( 1,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition(-1,  1);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition(-1, -1);
			blocks[1].setPosition( 0, -1);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition( 1, -1);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}

		blocks[0].setColor(sf::Color::White);
		blocks[1].setColor(sf::Color::White);
		blocks[2].setColor(sf::Color::White);
		blocks[3].setColor(sf::Color::White);
	}
	//S
	else if (shape == 5)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 1,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition( 0,  0);
			blocks[1].setPosition( 1,  0);
			blocks[2].setPosition(-1,  1);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition(-1, -1);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition( 1, -1);
			blocks[2].setPosition(-1,  0);
			blocks[3].setPosition( 0,  0);
		}

		blocks[0].setColor(sf::Color::Green);
		blocks[1].setColor(sf::Color::Green);
		blocks[2].setColor(sf::Color::Green);
		blocks[3].setColor(sf::Color::Green);
	}
	//Z
	else if (shape == 6)
	{
		if (rotation == 0)
		{
			blocks[0].setPosition( 1, -1);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 1,  0);
			blocks[3].setPosition( 0,  1);
		}
		else if (rotation == 1)
		{
			blocks[0].setPosition(-1,  0);
			blocks[1].setPosition( 0,  0);
			blocks[2].setPosition( 0,  1);
			blocks[3].setPosition( 1,  1);
		}
		else if (rotation == 2)
		{
			blocks[0].setPosition( 0, -1);
			blocks[1].setPosition(-1,  0);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition(-1,  1);
		}
		else if (rotation == 3)
		{
			blocks[0].setPosition(-1, -1);
			blocks[1].setPosition( 0, -1);
			blocks[2].setPosition( 0,  0);
			blocks[3].setPosition( 1,  0);
		}
		blocks[0].setColor(sf::Color::Red);
		blocks[1].setColor(sf::Color::Red);
		blocks[2].setColor(sf::Color::Red);
		blocks[3].setColor(sf::Color::Red);
	}
	if (blockList.size() == 0)
	{
		blockList.push_back(blocks[0]);
		blockList.push_back(blocks[1]);
		blockList.push_back(blocks[2]);
		blockList.push_back(blocks[3]);
	}
	else if (blockList.size() == 4)
	{
		blockList[0] = blocks[0];
		blockList[1] = blocks[1];
		blockList[2] = blocks[2];
		blockList[3] = blocks[3];
	}
}

//Return the whole block list of this tetromino
std::vector<block> tetromino::returnBlockList()
{
	return blockList;
}

//Returns whether or not the active tetromino can move in a specified direction
//0: Up, 1: Left, 2: Down, 3: Right
bool canMove(std::vector<tetromino> tetrominoList, std::vector<block> blockList, int direction)
{
	tetromino activeTet = tetrominoList[0];
	if (direction == 0)
	{
		//TODO eventually; currently a piece will never go up
		return false;
	}
	else if (direction == 1)
	{
		//Check border collision
		for (int i = 0; i < activeTet.returnBlockList().size(); i++)
		{
			if (activeTet.returnBlockList()[i].returnPosition().x + activeTet.returnPosition().x <= 0)
			{
				return false;
			}
		}
		//Check tetromino collision
		for (int i = 0; i < blockList.size(); i++)
		{
			for (int j = 0; j < activeTet.returnBlockList().size(); j++)
			{
				if ((activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x - 1 == blockList[i].returnPosition().x) && (activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y == blockList[i].returnPosition().y))
				{
					return false;
				}
			}
		}

		//Otherwise
		return true;
	}
	else if (direction == 2)
	{
		//Check border collision
		for (int i = 0; i < activeTet.returnBlockList().size(); i++)
		{
			if (activeTet.returnBlockList()[i].returnPosition().y + activeTet.returnPosition().y >= numRows - 1)
			{
				return false;
			}
		}
		//Check tetromino collision
		for (int i = 0; i < blockList.size(); i++)
		{
			for (int j = 0; j < activeTet.returnBlockList().size(); j++)
			{
				if ((activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y + 1 == blockList[i].returnPosition().y) && (activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x == blockList[i].returnPosition().x))
				{
					return false;
				}
			}
		}

		//Otherwise
		return true;
	}
	else if (direction == 3)
	{
		//Check border collision
		for (int i = 0; i < activeTet.returnBlockList().size(); i++)
		{
			if (activeTet.returnBlockList()[i].returnPosition().x + activeTet.returnPosition().x >= numColumns - 1)
			{
				return false;
			}
		}
		//Check tetromino collision
		for (int i = 0; i < blockList.size(); i++)
		{
			for (int j = 0; j < activeTet.returnBlockList().size(); j++)
			{
				if ((activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x + 1 == blockList[i].returnPosition().x) && (activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y == blockList[i].returnPosition().y))
				{
					return false;
				}
			}
		}

		//Otherwise
		return true;
	}
	//Called when an invalid direction is passed
	else
	{
		return false;
	}
}

//Returns whether or not the active tetromino can rotate in a specified direction
//1: Clockwise, -1: Counter-clockwise
bool canRotate(std::vector<tetromino> tetrominoList, std::vector<block> blockList, int direction)
{
	tetromino activeTet = tetrominoList[tetrominoList.size() - 1];
	activeTet.rotate(direction);
	for (int i = 0; i < blockList.size(); i++)
	{
		for (int j = 0; j < activeTet.returnBlockList().size(); j++)
		{
			//If the rotated block crosses a border
			if (activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x < 0 || activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x >= numColumns || activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y < 0 || activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y >= numRows)
			{
				return false;
			}
			//If the rotated block overlaps with any tetromino, return false
			if (activeTet.returnBlockList()[j].returnPosition().x + activeTet.returnPosition().x  == blockList[i].returnPosition().x  && activeTet.returnBlockList()[j].returnPosition().y + activeTet.returnPosition().y == blockList[i].returnPosition().y)
			{
				return false;
			}
		}
	}

	//Otherwise
	return true;
}

//Returns whether or not the given row has been filled and is ready to clear
bool isRowComplete(std::vector<block> blockList, int row)
{
	bool columnFilled[numColumns];
	bool isComplete = true;
	//tetrominoList.size()-1 so the active piece isn't included
	for (int i = 0; i < blockList.size(); i++)
	{
		if (blockList[i].returnPosition().y == row)
		{
			columnFilled[blockList[i].returnPosition().x] = true;
		}
	}

	for (int i = 0; i < numColumns; i++)
	{
		if (!columnFilled[i])
		{
			isComplete = false;
		}
	}

	return isComplete;
}

//Returns a new tetromino list in which a given row has been cleared
std::vector<block> clearRow(std::vector<block> blockList, int row)
{
	for (int i = 0; i < blockList.size(); i++)
	{
		if (blockList[i].returnPosition().y == row)
		{
			blockList.erase(blockList.begin() + i);
		}
		
		//Moves lines down
		if (blockList[i].returnPosition().y < row)
		{
			blockList[i].move(2);
		}
	}

	return blockList;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(windowX, windowY), "Tetris Clone");

	std::vector<std::vector<cell>> cellMap;
	std::vector<block> blockListActive;
	std::vector<block> decomposedTetrominos;
	std::vector<tetromino> tetrominoList;
	bool isPlaying = true;
	int score = 0;

	//Random number stuff
	std::random_device rd;
	std::default_random_engine generator;
	generator.seed(rd());
	std::uniform_int_distribution<int> randomPiece(0, 6);

	//Populates cell map
	for (int i = 0; i < numColumns; i++)
	{
		cellMap.push_back(std::vector<cell>());
		for (int j = 0; j < numRows; j++)
		{
			cell newCell(i, j);
			cellMap[i].push_back(newCell);
		}
	}

	//Tetromino list with first random tetromino pushed
	tetromino firstTet(randomPiece(generator));
	tetrominoList.push_back(firstTet);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Left && isPlaying && canMove(tetrominoList, decomposedTetrominos, 1))
				{
					tetrominoList[tetrominoList.size() - 1].move(1);
				}
				else if (event.key.code == sf::Keyboard::Right && isPlaying && canMove(tetrominoList,decomposedTetrominos, 3))
				{
					tetrominoList[tetrominoList.size() - 1].move(3);
				}
				else if (event.key.code == sf::Keyboard::Up && isPlaying && canRotate(tetrominoList,decomposedTetrominos, 1))
				{
					tetrominoList[tetrominoList.size() - 1].rotate(1);
				}
				else if (event.key.code == sf::Keyboard::Down && isPlaying && canMove(tetrominoList,decomposedTetrominos, 2))
				{
					tetrominoList[tetrominoList.size() - 1].move(2);
				}
				else if (event.key.code == sf::Keyboard::Space)
				{
					if (isPlaying)
					{
						isPlaying = false;
					}
					else
					{
						isPlaying = true;
					}
				}
			}
		}

		window.clear();
		
		if (isPlaying)
		{
			//Add tetromino list to displayed block list
			for (int i = 0; i < tetrominoList.size(); i++)
			{
				tetrominoList[i].configBlockList();
				for (int j = 0; j < tetrominoList[i].returnBlockList().size(); j++)
				{
					block tempBlock;
					tempBlock.setPosition(tetrominoList[i].returnBlockList()[j].returnPosition().x + tetrominoList[i].returnPosition().x, tetrominoList[i].returnBlockList()[j].returnPosition().y + tetrominoList[i].returnPosition().y);
					tempBlock.setColor(tetrominoList[i].returnBlockList()[j].returnColor());
					blockListActive.push_back(tempBlock);
				}
			}
			//Add decomposed tetrominos block list to displayed block list
			for (int i = 0; i < decomposedTetrominos.size(); i++)
			{
				blockListActive.push_back(decomposedTetrominos[i]);
			}
			
			//Set cells that correspond to each block in the displayed block list active
			for (int i = 0; i < blockListActive.size(); i++)
			{	
				if (blockListActive[i].returnPosition().x > -1 && blockListActive[i].returnPosition().x < numColumns && blockListActive[i].returnPosition().y > -1 && blockListActive[i].returnPosition().y < numRows)
				{
					cellMap[blockListActive[i].returnPosition().x][blockListActive[i].returnPosition().y].configFill(blockListActive[i].returnColor());
					cellMap[blockListActive[i].returnPosition().x][blockListActive[i].returnPosition().y].setIsFilled(true);
				}
			}
	
			//Move down each tick if it is possible
			if (canMove(tetrominoList, decomposedTetrominos, 2))
			{
				tetrominoList[tetrominoList.size() - 1].move(2);
			}
			//Else spawn a new tetromino and decompose the previous tetromino
			else
			{
				decomposedTetrominos = tetrominoList[0].decompose(decomposedTetrominos);
				tetrominoList.pop_back();
				tetromino newTet(randomPiece(generator));
				tetrominoList.push_back(newTet);
			}
			
			//Loss checking
			for (int i = 0; i < decomposedTetrominos.size(); i++)
			{
				if (decomposedTetrominos[i].returnPosition().x == numColumns / 2 && decomposedTetrominos[i].returnPosition().y == 0)
				{
					score = 0;
					decomposedTetrominos.clear();
				}
			}

			//Line checking

			std::vector<int> linesCleared;

			for (int i = 0; i < numRows; i++)
			{
				if (isRowComplete(decomposedTetrominos, i))
				{
					linesCleared.push_back(i);
				}
			}

			if (linesCleared.size() == 0)
			{
				//This will be true for the majority of the game so I am including it as the first of this if else-if change to save processing time
			}
			else if (linesCleared.size() == 1)
			{
				score += 40;
			}
			else if (linesCleared.size() == 2)
			{
				score += 100;
			}
			else if (linesCleared.size() == 3)
			{
				score += 300;
			}
			else if (linesCleared.size() == 4)
			{
				score += 1200;
			}

			while (linesCleared.size() > 0)
			{
				decomposedTetrominos = clearRow(decomposedTetrominos, linesCleared[0]);
				linesCleared.erase(linesCleared.begin());
			}
			
			//Draw filled cells
			for (int i = 0; i < numColumns; i++)
			{
				for (int j = 0; j < numRows; j++)
				{
					for (int k = 0; k < 4; k++)
					{
						window.draw(cellMap[i][j].returnLine(k));
					}

					if (cellMap[i][j].returnIsFilled())
					{
						window.draw(cellMap[i][j].returnFill());
					}
				}
			}

			std::cout << score << std::endl;
		}

		window.display();

		for (int i = 0; i < numColumns; i++)
		{
			for (int j = 0; j < numRows; j++)
			{
				cellMap[i][j].setIsFilled(false);
			}
		}

		blockListActive.clear();

		//Tick timer
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
}
