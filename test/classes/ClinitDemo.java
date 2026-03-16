public class ClinitDemo extends Parent {
    static {
        System.out.println("Hello from <clinit>.");
    }

    public static void main(String[] args) {
        
    }
}

class Parent {
    static {
        System.out.println("Hello from <clinit> of Parent.");
    }
}
