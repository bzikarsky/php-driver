<?php

namespace Cassandra;

/**
 * @requires extension cassandra
 */
class DecimalTest extends \PHPUnit_Framework_TestCase
{
    /**
     * @expectedException         InvalidArgumentException
     * @expectedExceptionMessage  Unrecognized character 'q' at position 0
     */
    public function testThrowsWhenCreatingNotAnInteger()
    {
        new Decimal("qwe");
    }

    /**
     * @dataProvider validStrings
     */
    public function testCorrectlyParsesStrings($number, $value, $scale, $string)
    {
        $number = new Decimal($number);
        $this->assertEquals($value, $number->value());
        $this->assertEquals($scale, $number->scale());
        $this->assertEquals($string, (string) $number);
    }

    public function validStrings()
    {
        return array(
            array("123", "123", 0, "123"),
            array("0123", "83", 0, "83"),
            array("0x123", "291", 0, "291"),
            array("0b1010101", "85", 0, "85"),
            array("-123", "-123", 0, "-123"),
            array("-0123", "-83", 0, "-83"),
            array("-0x123", "-291", 0, "-291"),
            array("-0b1010101", "-85", 0, "-85"),
            array("1313123123.234234234234234234123", "1313123123234234234234234234123", 21, "1313123123.234234234234234234123"),
            array("123.1", "1231", 1, "123.1"),
            array("55.55", "5555", 2, "55.55"),
            array("-123.123", "-123123", 3, "-123.123"),
            array("0.5", "5", 1, "0.5")
        );
    }

    /**
     * @dataProvider validNumbers
     */
    public function testFromNumbers($number)
    {
        //$decimal = new Decimal($number);
        //$this->assertEquals((float)$number, (float)$decimal);
        //$this->assertEquals((float)$number, (float)(string)$decimal);
        //$this->assertEquals((int)$number, $decimal->toInt());
    }

    public function validNumbers()
    {
        return array(
            array(0.123),
            array(123),
        );
    }

    public function testAdd()
    {
        $decimal1 = new Decimal("1");
        $decimal2 = new Decimal("0.5");
        $this->assertEquals("1.5", (string)$decimal1->add($decimal2));
    }

    public function testSub()
    {
        $decimal1 = new Decimal("1");
        $decimal2 = new Decimal("0.5");
        $this->assertEquals("0.5", (string)$decimal1->sub($decimal2));
    }

    public function testMul()
    {
        $decimal1 = new Decimal("2");
        $decimal2 = new Decimal("0.5");
        $this->assertEquals("1.0", (string)$decimal1->mul($decimal2));
    }

    /**
     * @expectedException        RuntimeException
     * @expectedExceptionMessage Not implemented
     */
    public function testDiv()
    {
        $decimal1 = new Decimal("1.0");
        $decimal2 = new Decimal("0.5");
        $this->assertEquals("2.0", (string)$decimal1->div($decimal2));
    }

    /**
     * @expectedException        RuntimeException
     * @expectedExceptionMessage Not implemented
     */
    public function testDivByZero()
    {
        $decimal1 = new Decimal("1");
        $decimal2 = new Decimal("0");
        $decimal1->div($decimal2);
    }

    /**
     * @expectedException        RuntimeException
     * @expectedExceptionMessage Not implemented
     */
    public function testMod()
    {
        $decimal1 = new Decimal("1");
        $decimal2 = new Decimal("2");
        $decimal1->mod($decimal2);
    }

    public function testAbs()
    {
        $decimal1 = new Decimal("-123.123");
        $this->assertEquals("123.123", (string)$decimal1->abs());
    }

    public function testNeg()
    {
        $decimal1 = new Decimal("123.123");
        $this->assertEquals("-123.123", (string)$decimal1->neg());
    }

    /**
     * @expectedException        RuntimeException
     * @expectedExceptionMessage Not implemented
     */
    public function testSqrt()
    {
        $decimal = new Decimal("4");
        $decimal->sqrt();
    }
}
