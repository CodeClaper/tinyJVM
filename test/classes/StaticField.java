public class StaticField {
    private int a;
    private char b;
    private static int c = 3;

    public static void main(String[] args) {
        int d = c + 1;
        System.out.println("d: " + d);
    }
}
