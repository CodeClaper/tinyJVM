public class A implements I {
    private String name;
    
    public A() {
        this.name = "A";
    }

    public void sayHello() {
        System.out.println("Say Hello from: " + this.name);
    }
}
