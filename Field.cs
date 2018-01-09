using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;

namespace Smokin
{
    class Field
    {
        private int width = 0;
        private int height = 0;
        private int depth = 0;

        private double elapsed_time = 0;

        private Cube cube = null;
        private Effect cube_effect = null;
        private EffectParameter model_view_projection_parameter;
        private EffectParameter elapsed_time_parameter;

        private Vector3 corner;
        private Vector3 translation;

        public Field(int width, int height, int depth)
        {
            this.width = width;
            this.height = height;
            this.depth = depth;
        }

        public void initialize(Cube cube, GraphicsDeviceManager graphics)
        {
            this.cube = cube;
            // cube_effect = new BasicEffect(graphics.GraphicsDevice, null);

            
            corner = boundingBox().Min + new Vector3(cube.size / 2.0f,
                                                     cube.size / 2.0f,
                                                     cube.size / 2.0f);

            translation = new Vector3();
        }

        public void update(double time)
        {
            elapsed_time += time;
        }

        public void loadContent(ContentManager content)
        {
            cube_effect = content.Load<Effect>("FieldShader");
            model_view_projection_parameter = cube_effect.Parameters["world_view_projection"];
            elapsed_time_parameter = cube_effect.Parameters["elapsed_time"];
        }

        public BoundingBox boundingBox()
        {
            BoundingBox bounding_box = new BoundingBox();


            bounding_box.Min.X = -(width * cube.size) / 2.0f;
            bounding_box.Min.Y = -(height * cube.size) / 2.0f;
            bounding_box.Min.Z = -(depth * cube.size) / 2.0f;

            bounding_box.Max.X = (width * cube.size) / 2.0f;
            bounding_box.Max.Y = (height * cube.size) / 2.0f;
            bounding_box.Max.Z = (depth * cube.size) / 2.0f;

            return bounding_box;
        }

        public void render(GraphicsDeviceManager graphics, Matrix view_matrix, Matrix projection_matrix)
        {
            elapsed_time_parameter.SetValue((float)elapsed_time);

            
            for (int x = 0; x != width; ++x)
            {
                for (int y = 0; y != height; ++y)
                {
                    for (int z = 0; z != depth; ++z)
                    {
                        translation.X = cube.size * x;
                        translation.Y = cube.size * y;
                        translation.Z = cube.size * z;

                        Matrix model_matrix = Matrix.CreateTranslation(translation + corner);

                        //cube_effect.VertexColorEnabled = true;
                        //cube_effect.World = model_matrix;
                        //cube_effect.View = view_matrix;
                        //cube_effect.Projection = projection_matrix;

                        cube_effect.Begin();

                        model_view_projection_parameter.SetValue(model_matrix * view_matrix * projection_matrix);

                     
                        foreach (EffectPass pass in cube_effect.CurrentTechnique.Passes)
                        {
                            pass.Begin();

                            cube.render(graphics);

                            pass.End();
                        }

                        cube_effect.End();

                    }
                }
            }

        }
    }
}
