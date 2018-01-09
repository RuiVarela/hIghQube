using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework;

namespace Smokin
{
    class Camera
    {
   

        /*

          private int window_width = 0;
          private int window_height = 0;
         
         
           private float yaw_ = 0;
           private float pitch_ = 0;
           private float roll_ = 0;

           //
           // Translation
           //
           private int x_movement_begin_drag = 0;
           private int y_movement_begin_drag = 0;
           private int z_movement_begin_drag = 0;
           private bool drag_movement_on = false;
           private bool drag_movement_z_on = false;

           private float drag_movement_increase = 1.0f;
           private float drag_movement_decay = 0.8f;
           private float x_drag_movement_power = 0.0f;
           private float y_drag_movement_power = 0.0f;
           private float z_drag_movement_power = 0.0f;

           //
           // Rotation
           //
           private int x_rotation_begin_drag = 0;
           private int y_rotation_begin_drag = 0;
           private bool drag_rotation_on = false;
           private float drag_rotation_increase = 1.0f;
           private float drag_rotation_decay = 0.7f;
           private float yaw_drag_rotation_power = 0.0f;
           private float pitch_drag_rotation_power = 0.0f;

   */

        // physics
        private Vector3 position_ = new Vector3(0.0f, 0.0f, 0.0f);
        private Vector3 desired_position_ = new Vector3(0.0f, 0.0f, 0.0f);
        private Vector3 position_velocity = new Vector3(0.0f, 0.0f, 0.0f);

        private float damping = 600.0f;
        private float mass = 50.0f;
        private float stiffness = 180.0f;

        private Vector3 up_vector = new Vector3(0.0f, 1.0f, 0.0f);

        private Vector3 target_position = new Vector3(0.0f, 1.0f, 0.0f);
        private Vector3 desired_target_position = new Vector3(0.0f, 1.0f, 0.0f);
        private Vector3 target_position_velocity = new Vector3(0.0f, 0.0f, 0.0f);

        public Vector3 desiredTargetPosition
        {
            get { return desired_target_position; }
            set { desired_target_position = value; }
        }


        public Vector3 upVector
        {
            get
            {
                return up_vector;
            }
            set
            {
                up_vector = value;
            }
        }

        public Vector3 targetPosition
        {
            get
            {
                return target_position;
            }
            set
            {
                target_position = value;
            }
        }

        public Vector3 desiredPosition
        {
            get
            {
                return desired_position_;
            }
            set
            {
                desired_position_ = value;
            }
        }

        public Vector3 position
        {
            get
            {
                return position_;
            }
            set
            {
                desired_position_ = value;
                position_ = value;
            }
        }


        public Camera()
        {
        }

        public void initialize(int width, int height)
        {
           // window_width = width;
           // window_height = height;
        }

        private void updatePhysics(double time)
        {

            // Calculate spring force
            Vector3 stretch = position - desiredPosition;
            Vector3 force = -stiffness * stretch - damping * position_velocity;

            // Apply acceleration
            Vector3 acceleration = force / mass;
            position_velocity += acceleration * (float)time;

            // Apply velocity
            position += position_velocity * (float)time;



            stretch = target_position - desired_target_position;
            force = -stiffness * stretch - damping * target_position_velocity;

            // Apply acceleration
            acceleration = force / mass;
            target_position_velocity += acceleration * (float)time;

            // Apply velocity
            target_position += target_position_velocity * (float)time;

        }

        public void update(double time)
        {
            /*
            // translation

            float relative_x_drag = 0;
            float relative_y_drag = 0;

            if (Mouse.GetState().LeftButton == ButtonState.Pressed)
            {
                if (drag_movement_on)
                {
                    relative_x_drag = ((float)(x_movement_begin_drag - Mouse.GetState().X) / (float)window_width);
                    relative_y_drag = ((float)(y_movement_begin_drag - Mouse.GetState().Y) / (float)window_height);
                }

                x_movement_begin_drag = Mouse.GetState().X;
                y_movement_begin_drag = Mouse.GetState().Y;

                drag_movement_on = true;
            }
            else
            {
                drag_movement_on = false;
            }
         
            x_drag_movement_power += -relative_x_drag * drag_movement_increase;
            y_drag_movement_power += relative_y_drag * drag_movement_increase;

            // z
            relative_y_drag = 0;

            if (Mouse.GetState().MiddleButton == ButtonState.Pressed)
            {
                if (drag_movement_z_on)
                {
                    relative_y_drag = ((float)(z_movement_begin_drag - Mouse.GetState().Y) / (float)window_width);
                }

                z_movement_begin_drag = Mouse.GetState().Y;

                drag_movement_z_on = true;
            }
            else
            {
                drag_movement_z_on = false;
            }

            z_drag_movement_power -= relative_y_drag * drag_movement_increase;

            
            x_drag_movement_power *= drag_movement_decay;
            y_drag_movement_power *= drag_movement_decay;
            z_drag_movement_power *= drag_movement_decay;

            Vector3 movement = new Vector3(x_drag_movement_power,
                                           z_drag_movement_power,
                                           y_drag_movement_power);

 
            Matrix matrix = Matrix.CreateFromYawPitchRoll(yaw, pitch, roll);
            matrix = Matrix.Invert(matrix);
                      
            
            position += Vector3.Transform(movement, matrix);

            // rotation

            relative_x_drag = 0;
            relative_y_drag = 0;


            if (Mouse.GetState().RightButton == ButtonState.Pressed)
            {
                if (drag_rotation_on)
                {
                    relative_x_drag = ((float)(x_rotation_begin_drag - Mouse.GetState().X) / (float)window_width);
                    relative_y_drag = ((float)(y_rotation_begin_drag - Mouse.GetState().Y) / (float)window_height);
                }

                x_rotation_begin_drag = Mouse.GetState().X;
                y_rotation_begin_drag = Mouse.GetState().Y;

                drag_rotation_on = true;
            }
            else
            {
                drag_rotation_on = false;
            }
        
            yaw_drag_rotation_power -= relative_x_drag * drag_rotation_increase;
            pitch_drag_rotation_power -= relative_y_drag * drag_rotation_increase;

            yaw_drag_rotation_power *= drag_rotation_decay;
            pitch_drag_rotation_power *= drag_rotation_decay;


            yaw += yaw_drag_rotation_power;
            pitch += pitch_drag_rotation_power;
            */



            updatePhysics(time);
        }

        //public Matrix viewMatrix()
        //{
        //    return Matrix.CreateTranslation(position) *
        //           Matrix.CreateFromYawPitchRoll(yaw, pitch, roll);
        //}

        public Matrix viewMatrix()
        {
            return Matrix.CreateLookAt(position, target_position, up_vector);
        }



    }
}
