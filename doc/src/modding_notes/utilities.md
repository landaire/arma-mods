# Tips and Utilities

## Utilities

#### Drawing Debug Shapes

Debug shapes can be drawn from any bit of code. Quick and easy method:

```c#
// class var
protected ref array<ref Shape> m_aDebugShapes = {};

// In some function

// Drop previous shapes (they go away with GC)
m_aDebugShapes.Clear();
vector shapePoints[2] = {
    start,
    end,
};

int color = Color.Blue.PackToInt();
ShapeFlags shapeFlags = SHAPEFLAGS.NOZBUFFER | SHapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;

m_aDebugShapes.Insert(Shape.CreateLines(color, shapeFlags, shapePoints, 2));
```