using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;


namespace Smokin
{
    class CubeLevel
    {
        enum CubeState
        {
            Empty,
            Neutral,
            Good,
            Evil
        };

        public int start_position = 0;
        public const int  number_of_columns = 5;
        public const int number_of_rows = 10;
        public const int field_size = 15;

        private float rest_time = 2.0f;
        private float rotation_time = 1.5f;

        private CubeState[,] numbers = null;

        private double elapsed_time = 0;
        private Cube cube = null;
        private Effect cube_effect = null;

        private EffectParameter model_parameter;
        private EffectParameter view_parameter;
        private EffectParameter projection_parameter;
        private EffectParameter camera_position;

        private EffectParameter elapsed_time_parameter;
        private EffectParameter color_parameter;
        private EffectParameter scale_parameter;

        private Field field = new Field(number_of_columns, 1, number_of_rows + field_size);


        private Vector4 neutral_color = new Vector4(0.2f, 0.2f, 0.0f, 1.0f);
        private Vector4 good_color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);
        private Vector4 evil_color = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        private Vector3 corner;
        private Vector3 position = new Vector3();
        private Matrix model_matrix = new Matrix();
        private float current_rotation_factor;

        // avatar
        private Pyramid pyramid;
        private Vector4 avatar_color = new Vector4(0.8f, 0.0f, 0.2f, 1.0f);
        private int avatar_x = 2;
        private int avatar_z = 20;
        private float avatar_rotation_speed = 1.0f;

        //input
        private double input_interval = 0.2;
        private double last_up = 0;
        private double last_down = 0;
        private double last_left = 0;
        private double last_right = 0;
        private double last_select = 0;
        private double last_destroy = 0;

        private List<Vector2> selection = new List<Vector2>();
        private Vector4 selection_color = new Vector4(0.2f, 0.1f, 0.4f, 1.0f);
        private float explosion_time = 0.5f;
        private Vector4 explosion_color = new Vector4(1.0f, 0.5f, 0.2f, 1.0f);

        public int moves = 0;
        public int good_detonations = 0;
        public int bad_detonations = 0;


        private bool rumble_active = false;
        private float rumble_start = 0;
        private float rumble_duration = 0.5f;

        class CubeExplosion
        {
            public float x;
            public float y;
            public float z;
            public float scale;
            public double trigger_time;
        };

        private List<CubeExplosion> explosion_list = new List<CubeExplosion>();


        // explosion

        internal Field Field
        {
            get { return field; }
            set { field = value; }
        }

        public CubeLevel()
        {
            numbers = new CubeState[number_of_rows, number_of_columns] 
            { 
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Good,       CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Evil,      CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Evil,     CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Good,       CubeState.Neutral, CubeState.Neutral, CubeState.Neutral, CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Neutral, CubeState.Good,       CubeState.Neutral },
                { CubeState.Neutral,    CubeState.Neutral, CubeState.Evil,      CubeState.Neutral, CubeState.Neutral }
            };
        }

        public Vector3 avatarPosition()
        {
            Vector3 avatar_position = new Vector3(avatar_x * cube.size, 0.0f, avatar_z * cube.size);

            return avatar_position + corner;
        }

        public void initialize(Cube cube, GraphicsDeviceManager graphics)
        {
            pyramid = new Pyramid();
            pyramid.initialize(1.0f, graphics);

            this.cube = cube;
            field.initialize(cube, graphics);

           corner = new Vector3(field.boundingBox().Min.X,
                                  field.boundingBox().Max.Y,
                                  field.boundingBox().Min.Z)
                                  +
                     new Vector3(cube.size / 2.0f,
                                 cube.size / 2.0f,
                                 cube.size / 2.0f);

        }


        public void loadContent(Microsoft.Xna.Framework.Game game)
        {
            field.loadContent(game.Content);

            cube_effect = game.Content.Load<Effect>("CubeLevel");


            model_parameter = cube_effect.Parameters["model"];
            view_parameter = cube_effect.Parameters["view"];
            projection_parameter = cube_effect.Parameters["projection"];

            elapsed_time_parameter = cube_effect.Parameters["elapsed_time"];
            color_parameter = cube_effect.Parameters["color"];
            scale_parameter = cube_effect.Parameters["scale"];

            camera_position = cube_effect.Parameters["camera_position"];


        }


        public bool findSelectionByPostion(Vector2 test)
        {

            /*List<SomeObject> myObjects = new List<SomeObject>();
             .. load objects up somehow .. 
            SomeObject desiredObject =
                myObjects.Find(delegate(SomeObject o) { return o.Id == desiredId; });
            */
            
            foreach (Vector2 vector in selection)
            {
                if (vector.Equals(test))
                {
                    return true;
                }
            }

            return false;
        }

        public void removeSelectionByPosition(Vector2 test)
        {
            int i = 0;
            while (i < selection.Count)
            {
                if (selection[i].Equals(test))
                {
                    selection.RemoveAt(i);
                }
                else
                {
                    i++;
                }
            }
        }

        public bool positionExists(int x, int z)
        {
            return (((x >= 0) && (x < number_of_columns)) &&
                   ((z >= 0) && (z < number_of_rows + field_size)));
        }

        public bool canChangeToPosition(int x, int z)
        {
            if (!positionExists(x, z))
            {
                return false;
            }

            // user inside the collision zone?

            int collision_zone_begin = start_position;
            int collision_zone_end = start_position + number_of_rows;

            if ( (z >= collision_zone_begin) &&
                 (z < collision_zone_end))
            {
                return (numbers[z - collision_zone_begin, x] == CubeState.Empty);
            }

            // we are rotating... so, test next collision
            if (current_rotation_factor != 0.0)
            {
                collision_zone_begin = start_position + 1;
                collision_zone_end = start_position + 1 + number_of_rows;

                if ((z >= collision_zone_begin) &&
                     (z < collision_zone_end))
                {
                    return (numbers[z - collision_zone_begin, x] == CubeState.Empty);
                }


            }


            return true;
        }

        public void selectDestroyPosition(int x, int z, List<Vector2> selection_list)
        {
            int next_x = x + 1;
            int next_z = z + 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x - 1;
            next_z = z - 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x + 1;
            next_z = z - 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x - 1;
            next_z = z + 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));

            }

            next_x = x;
            next_z = z;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }


            next_x = x;
            next_z = z + 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x;
            next_z = z - 1;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x + 1;
            next_z = z;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }

            next_x = x - 1;
            next_z = z;

            if (positionExists(next_x, next_z))
            {
                selection_list.Add(new Vector2(next_x, next_z));
            }
        }

        public void selectionDestroy(double time)
        {
            List<Vector2> temporary_selection = new List<Vector2>();

            for (int i = 0; i != number_of_rows; ++i)
            {
                for (int j = 0; j != number_of_columns; ++j)
                {
                    foreach (Vector2 vector in selection)
                    {

                        if ((vector.X == j) && (vector.Y == start_position + i))
                        {
                            if (numbers[i, j] == CubeState.Evil)
                            {
                                ++bad_detonations;
                            }

                            if (numbers[i, j] == CubeState.Neutral)
                            {
                                ++good_detonations;
                            }

                            if (numbers[i, j] == CubeState.Good)
                            {
                                ++good_detonations;
                                selectDestroyPosition((int)vector.X, (int)vector.Y, temporary_selection);
                            }
                                                       

                            CubeExplosion explosion = new CubeExplosion();
                            explosion.x = j * cube.size;
                            explosion.y = 0.0f;
                            explosion.z = (i + start_position) * cube.size;
                            explosion.trigger_time = time;
                            explosion.scale = 1.1f;
                            explosion_list.Add(explosion);
                            

                            numbers[i, j] = CubeState.Empty;

                            if (GamePad.GetState(PlayerIndex.One).IsConnected)
                            {
                                GamePad.SetVibration(PlayerIndex.One, 1.0f, 1.0f);
                                rumble_start = (float)elapsed_time;
                                rumble_active = true;
                            }
                                                        
                        }
                    }

                }
            }

 
            selection.Clear();

            foreach (Vector2 vector in temporary_selection)
            {
                selection.Add(vector);
             }
        }

        void updateExplosions()
        {
            int i = 0;
            while (i < explosion_list.Count)
            {
                if ((explosion_list[i].trigger_time + explosion_time) < elapsed_time)
                {
                    explosion_list.RemoveAt(i);
                }
                else
                {
                    explosion_list[i].scale = 1.0f  + ((float)(elapsed_time - explosion_list[i].trigger_time) / explosion_time);;

                    i++;
                }
            }



        } 

        public void update(double time)
        {
            elapsed_time += time;
            field.update(time);

            float fraction_part = (float) elapsed_time / (rotation_time + rest_time);
            start_position = (int)Math.Floor(fraction_part);

            fraction_part -= (int)fraction_part;

            double modulated_time = (fraction_part * (rotation_time + rest_time));

            if (modulated_time < rest_time)
            {
                current_rotation_factor = 0.0f;
            }
            else
            {
                modulated_time -= rest_time;
                
                current_rotation_factor = (float) modulated_time / rotation_time;
            }


            bool up_pressed      = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.DPadUp) : Keyboard.GetState().IsKeyDown(Keys.Up);
            bool down_pressed    = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.DPadDown) : Keyboard.GetState().IsKeyDown(Keys.Down);
            bool left_pressed    = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.DPadLeft) : Keyboard.GetState().IsKeyDown(Keys.Left);
            bool right_pressed   = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.DPadRight) : Keyboard.GetState().IsKeyDown(Keys.Right);
            bool space_pressed   = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.A) : Keyboard.GetState().IsKeyDown(Keys.Space);
            bool destroy_pressed = GamePad.GetState(PlayerIndex.One).IsConnected ? GamePad.GetState(PlayerIndex.One).IsButtonDown(Buttons.B) : Keyboard.GetState().IsKeyDown(Keys.B);


            if (up_pressed)
            {
                if (canChangeToPosition(avatar_x, avatar_z - 1) &&
                     (elapsed_time > (last_up + input_interval)))
                {
                    avatar_z--;
                    last_up = elapsed_time;
                }
            }

            if (down_pressed)
            {
                if (canChangeToPosition(avatar_x, avatar_z + 1) &&
                     (elapsed_time > (last_down + input_interval)) )
                {
                    avatar_z++;
                    last_down = elapsed_time;
                }
            }

            if (left_pressed)
            {
                if (canChangeToPosition(avatar_x - 1, avatar_z) &&
                     (elapsed_time > (last_left + input_interval)))
                {
                    avatar_x--;
                    last_left = elapsed_time;
                }
            }

            if (right_pressed)
            {
                if (canChangeToPosition(avatar_x + 1, avatar_z) &&
                     (elapsed_time > (last_right + input_interval)))
                {
                    avatar_x++;
                    last_right = elapsed_time;
                }
            }

            if (destroy_pressed)
            {
                if ((elapsed_time > (last_destroy + input_interval))
                    && (current_rotation_factor == 0.0))
                {
                    selectionDestroy(elapsed_time);
                    last_destroy = elapsed_time;
                }
            }


            if (space_pressed)
            {
                if (elapsed_time > (last_select + input_interval))
                {
                    if (findSelectionByPostion(new Vector2(avatar_x, avatar_z)))
                    {
                        ++moves;
                        removeSelectionByPosition(new Vector2(avatar_x, avatar_z));
                        
                    }
                    else
                    {
                        ++moves;
                        selection.Add(new Vector2(avatar_x, avatar_z));
                       
                    }
                    last_select = elapsed_time;
                }
            }

            updateExplosions();

            if (rumble_active)
            {
                if ((rumble_start + rumble_duration) < elapsed_time)
                {
                    GamePad.SetVibration(PlayerIndex.One, 0.0f, 0.0f);
                    rumble_active = false;
                }
            }
        }


        public void render(GraphicsDeviceManager graphics, Camera camera, Matrix view_matrix, Matrix projection_matrix)
        {
            view_parameter.SetValue(view_matrix);
            projection_parameter.SetValue(projection_matrix);
            camera_position.SetValue(camera.position);
            scale_parameter.SetValue(1.0f);

            // Render field
            field.render(graphics, view_matrix, projection_matrix);

            // Render main cubes
            for (int i = 0; i != number_of_rows; ++i)
            {
                for (int j = 0; j != number_of_columns; ++j)
                {
                    if (numbers[i, j] != CubeState.Empty)
                    {
                        position.X = j * cube.size;
                        position.Y = 0.0f;
                        position.Z = (i + start_position) * cube.size
                                      + current_rotation_factor * cube.size;

                        model_matrix = Matrix.CreateFromYawPitchRoll(0.0f, (float)(Math.PI / 2.0f) * current_rotation_factor, 0.0f) *
                                       Matrix.CreateTranslation(position + corner);


                        elapsed_time_parameter.SetValue((float)elapsed_time);

                        model_parameter.SetValue(model_matrix);
 
                        if (numbers[i, j] == CubeState.Good)
                        {
                            color_parameter.SetValue(good_color);
                        }
                        else if (numbers[i, j] == CubeState.Evil)
                        {
                            color_parameter.SetValue(evil_color);
                        }
                        else if (numbers[i, j] == CubeState.Neutral)
                        {
                            color_parameter.SetValue(neutral_color);
                        }
                        
                        cube_effect.Begin();

                        // Normal Render Pass
                        graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullClockwiseFace;
                        EffectPass pass = cube_effect.CurrentTechnique.Passes[0];
                        pass.Begin();
                        cube.render(graphics);
                        pass.End();

                        // Outline pass
                        graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
                        pass = cube_effect.CurrentTechnique.Passes[1];
                        pass.Begin();
                        cube.render(graphics);
                        pass.End();
                        cube_effect.End();
                    }
                }
            }

            // Render avatar

          //  Vector3 avatar_position = new Vector3(avatar_x * cube.size, 0.0f, avatar_z * cube.size);

            model_matrix = Matrix.CreateFromYawPitchRoll(avatar_rotation_speed * (float) elapsed_time, 0.0f, 0.0f) *
                           Matrix.CreateTranslation(avatarPosition());

            model_parameter.SetValue(model_matrix);
            color_parameter.SetValue(avatar_color);
            

            cube_effect.Begin();
            // Normal Render Pass
            graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullClockwiseFace;
            EffectPass avatar_pass = cube_effect.CurrentTechnique.Passes[0];
            avatar_pass.Begin();
            pyramid.render(graphics);
            avatar_pass.End();

            // Outline pass
            graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
            avatar_pass = cube_effect.CurrentTechnique.Passes[1];
            avatar_pass.Begin();
            pyramid.render(graphics);
            avatar_pass.End();
            cube_effect.End();


            // render selection
            scale_parameter.SetValue(0.95f);

            foreach (Vector2 vector in selection)
            {
                model_matrix = Matrix.CreateFromYawPitchRoll(avatar_rotation_speed * (float)elapsed_time + (float)(Math.PI / 4.0f), 0.0f, 0.0f) *
                               Matrix.CreateTranslation(new Vector3(vector.X * cube.size, 0.0f, vector.Y * cube.size) + corner);

                model_parameter.SetValue(model_matrix);
                color_parameter.SetValue(selection_color);


                cube_effect.Begin();
                // Normal Render Pass
                graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullClockwiseFace;
                EffectPass selection_pass = cube_effect.CurrentTechnique.Passes[0];
                selection_pass.Begin();
                pyramid.render(graphics);
                selection_pass.End();

                // Outline pass
                graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
                selection_pass = cube_effect.CurrentTechnique.Passes[1];
                selection_pass.Begin();
                pyramid.render(graphics);
                selection_pass.End();
                cube_effect.End();
            }

            // render explosion

            
            color_parameter.SetValue(explosion_color);
            graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullClockwiseFace;

            foreach (CubeExplosion explosion in explosion_list)
            {
                model_matrix = Matrix.CreateTranslation(new Vector3(explosion.x, explosion.y, explosion.z) + corner);

                scale_parameter.SetValue(explosion.scale);
                model_parameter.SetValue(model_matrix);
                color_parameter.SetValue(selection_color);


                cube_effect.Begin();

                // Normal Render Pass
                
                EffectPass explosion_pass = cube_effect.CurrentTechnique.Passes[2];
                explosion_pass.Begin();
                cube.render(graphics);
                explosion_pass.End();

                cube_effect.End();

            }
        }
    }
}
