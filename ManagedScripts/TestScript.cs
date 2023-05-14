using ScriptAPI;

public class TestScript : Script
{
    public override void Update()
    {
        TransformComponent tf = GetTransformComponent();
        tf.X += 1.0f;
        Console.WriteLine($"x: {tf.X}");
    }
}