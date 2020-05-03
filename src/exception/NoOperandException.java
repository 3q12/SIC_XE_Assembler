package exception;

public class NoOperandException extends Exception{

    public NoOperandException(String message) {
        super(message);
    }

    public NoOperandException(String message, Throwable cause) {
        super(message, cause);
    }

}
