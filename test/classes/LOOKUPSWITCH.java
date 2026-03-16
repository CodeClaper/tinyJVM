public class LOOKUPSWITCH {
    public void testLookup(int key) {
        switch (key) {
            case 1:
                System.out.println("One");
                break;
            case 100:
                System.out.println("One Hundred");
                break;
            case 5000:
                System.out.println("Five Thousand");
                break;
            default:
                System.out.println("Other");
        }
    }
}
