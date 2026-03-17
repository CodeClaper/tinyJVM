public class B extends A  {
    private int a;
    private float b;
    private double c;
    private String name;
    private static String mes = "Hello world";
    
    public B() {
        super();
        this.a = 12345;
        this.b = 2.3f;
        this.c = 1023.33;
        this.name = "B";
    }

    @Override
    public void sayHello() {
        this.a++;
        this.b++;
        System.out.println("Say Hello from: " + this.name);
    }

    public void run(Runnable r) {
        r.run();
    }
}
