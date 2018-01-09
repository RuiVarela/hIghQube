using System;
using System.Collections.Generic;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Net;
using Microsoft.Xna.Framework.Storage;

using Smokin;

namespace Smokin
{

    public class hIghQube : Microsoft.Xna.Framework.Game
    {
        private int window_width = 1024;
        private int window_height = 768;
        private bool fullscreen = true;

        private Matrix projection_matrix;
        private Matrix view_matrix;
        private Matrix model_matrix;
                
        private GraphicsDeviceManager graphics;
                
        private Camera camera = new Camera();
        private Vector3 camera_offset = new Vector3(0.0f, 8.0f, 10.0f);
        private Cube cube = new Cube();
        private CubeLevel cube_level = new CubeLevel();

        SpriteFont font;
        SpriteBatch sprite_batch;

        public hIghQube()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        protected override void Initialize()
        {
            base.Initialize();
                      

            projection_matrix = Matrix.CreatePerspectiveFieldOfView(
                                MathHelper.ToRadians(45f),
                                (window_width / window_height), 0.01f, 100.0f);

            model_matrix = Matrix.Identity;


            graphics.PreferredBackBufferWidth = window_width;
            graphics.PreferredBackBufferHeight = window_height;
            graphics.MinimumPixelShaderProfile = ShaderProfile.PS_2_0;
            graphics.IsFullScreen = fullscreen;
            Window.Title = "hIgh Qube Game 0.0.1";

  
            graphics.ApplyChanges();


            camera.initialize(window_width, window_height);
            cube.initialize(1.0f, graphics);
            cube_level.initialize(cube, graphics);

            camera.upVector = new Vector3(0.0f, 1.0f, 0.0f);
            camera.position = new Vector3(0.0f, 0.0f, cube_level.Field.boundingBox().Min.Z) + camera_offset;

          //  camera.pitch = MathHelper.Pi / 8.0f;



            graphics.GraphicsDevice.RenderState.ColorWriteChannels = ColorWriteChannels.All;
            
            graphics.GraphicsDevice.RenderState.AlphaBlendEnable = true;
            graphics.GraphicsDevice.RenderState.SourceBlend = Blend.SourceAlpha;
            graphics.GraphicsDevice.RenderState.DestinationBlend = Blend.InverseSourceAlpha;

            graphics.GraphicsDevice.RenderState.CullMode = CullMode.CullClockwiseFace;
            graphics.GraphicsDevice.RenderState.DepthBufferEnable = true;

        }


        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            sprite_batch = new SpriteBatch(GraphicsDevice);
            cube_level.loadContent(this);

            font = Content.Load<SpriteFont>("Smokin");
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            double delta_time = (gameTime.ElapsedGameTime.Milliseconds / 1000.0f);

            camera.desiredTargetPosition = cube_level.avatarPosition();
            camera.desiredPosition = cube_level.avatarPosition() + camera_offset;
                        
            cube_level.update(delta_time);
            camera.update(delta_time);
            
            // Allows the game to exit
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed)
            {
                this.Exit();
            }
    
            model_matrix = Matrix.Identity;

            base.Update(gameTime);

        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
         // BeginDraw();


            graphics.GraphicsDevice.RenderState.PointSpriteEnable = false;


            // Set the alpha blend mode.
            graphics.GraphicsDevice.RenderState.AlphaBlendEnable = true;
            //renderState.AlphaBlendOperation = BlendFunction.Add;
  
            graphics.GraphicsDevice.RenderState.SourceBlend = Blend.SourceAlpha;
            graphics.GraphicsDevice.RenderState.DestinationBlend = Blend.InverseSourceAlpha;

            // Set the alpha test mode.
            graphics.GraphicsDevice.RenderState.AlphaTestEnable = true;
            graphics.GraphicsDevice.RenderState.AlphaFunction = CompareFunction.Greater;
            graphics.GraphicsDevice.RenderState.ReferenceAlpha = 0;

            // Enable the depth buffer (so particles will not be visible through
            // solid objects like the ground plane), but disable depth writes
            // (so particles will not obscure other particles).
            graphics.GraphicsDevice.RenderState.DepthBufferEnable = true;
            graphics.GraphicsDevice.RenderState.DepthBufferWriteEnable = true;

            

            graphics.GraphicsDevice.Clear(Color.Black);

            view_matrix = camera.viewMatrix();

            cube_level.render(graphics, camera, view_matrix, projection_matrix);

            base.Draw(gameTime);

            sprite_batch.Begin();
            sprite_batch.DrawString(font, "G: " + cube_level.good_detonations + 
                                          " B: " + cube_level.bad_detonations + 
                                          " Moves: " + cube_level.moves
                                          , new Vector2(0, 0), Color.Aqua);
            sprite_batch.End();

         //  EndDraw();
        }

        
    }
}
