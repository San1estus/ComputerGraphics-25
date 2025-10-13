#version 330 core
in vec3 v_Normal;
in vec3 v_FragPos;
out vec4 color;

uniform vec3 u_LightPos;  // Posición de la fuente de luz
uniform vec3 u_ViewPos;   // Posición de la cámara
uniform vec3 u_ObjectColor; // Color base de la esfera

void main()
{
    // 1. Iluminación Ambiental (Siempre hay un poco de luz)
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * u_ObjectColor;

    // 2. Iluminación Difusa (Cálculo de la luz directa)
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_ObjectColor;

    // 3. Iluminación Especular (Brillo, ignorada por simplicidad, pero se puede añadir)

    // 4. Color Final
    vec3 result = (ambient + diffuse);
    color = vec4(result, 1.0f);
}