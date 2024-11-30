#version 330 core

out vec4 fragColor;
in vec2 fragCoord;

uniform vec2 iResolution;

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord.xy / iResolution.xy; // Нормализуем координаты фрагмента
    vec2 grid = floor(uv * vec2(10.0)); // Размер ячейки сетки 10x10 пикселей

    int gridX = int(grid.x);
    int gridY = int(grid.y);

    if ((gridX + gridY) % 2 == 0)
    {
        fragColor = vec4(0.827, 0.988, 0.498, 1.0); // #d3fc7e
    }
    else
    {
        fragColor = vec4(0.600, 0.902, 0.373, 1.0); // #99e65f
    }
}

void main()
{
    mainImage(fragColor, gl_FragCoord.xy);
}
