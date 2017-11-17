#pragma once

/*
 * Defines the grid.
 */
class Grid
{
public:
	Grid( size_t dim );
	~Grid();

	/* Resets the heights of the grid. */
	void reset();

	/* Resets the heights of the grid. */
	void reset(int colour);

	/*  Gets the dimensions of the grid. */
	size_t getDim() const;

	/*  Gets the height of a grid at the specified position. */
	int getHeight( int x, int y ) const;

	/*  Gets the colour of the grid at the specified position. */
	int getColour( int x, int y ) const;

	/*  Sets the height of a grid at the specified position. */
	void setHeight( int x, int y, int h );

	/*  Sets the colour of a grid at the specified position. */
	void setColour( int x, int y, int c );

private:
	size_t m_dim;
	int *m_heights;
	int *m_cols;
};
