import java.util.concurrent.Semaphore;
public class TrafficController implements Runnable{
    private Semaphore sem;
    private int cars;
    public TrafficController() {
        this.sem = new Semaphore(1, true);
        this.cars=0;
    }

    public void run(){

    }

    public void enterLeft() {
        try {
            System.out.println("There's a RED waiting.");
            sem.acquire();
            cars++;
            System.out.println("RED  ("+cars+") is at the bridge.");
        } catch (InterruptedException ex) {
            System.out.println(ex);
        }
    }

    public void enterRight() {
        try {
            System.out.println("There's a BLUE waiting.");
            sem.acquire();
            cars++;
            System.out.println("BLUE ("+cars+") is at the bridge.");
        } catch (InterruptedException ex) {
            System.out.println(ex);
        }
    }

    public void leaveLeft() {
        System.out.println("BLUE ("+cars+") passed through the bridge.");
        sem.release();
    }

    public void leaveRight() {
        System.out.println("RED  ("+cars+") passed through the bridge.");
        sem.release();
    }
}
