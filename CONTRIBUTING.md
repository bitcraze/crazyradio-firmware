Contributing
============

👍🎉 Thanks a lot for considering contributing 🎉👍

We welcome and encourage contribution. There is many way to contribute: you can
write bug report, contribute code or documentation.
You can also go to the [bitcraze forum](https://forum.bitcraze.io) and help others.

## Reporting issues

When reporting issues the more information you can supply the better.

 - **Information about the environment:**
   - What version of the firmware are you running
   - What OS are you running on your computer
 - **How to reproduce the issue:** Step-by-step guide on how the issue can be reproduced (or at least how you reproduce it).
 Include everything you think might be useful, the more information the better.

## Improvements request and proposal

Feel free to make an issue to request a new functionality.

## Contributing code/Pull-Request

We welcome code contribution, this can be done by starting a pull-request.

If the change is big, typically if the change span to more than one file, consider starting an issue first to discuss the improvement.
This will makes it much easier to make the change fit well into the firmware.

There is some basic requirement for us to merge a pull request:
 - Describe the change
 - Refer to any issues it effects
 - Separate one pull request per functionality: if you start writing "and" in the feature description consider if it could be separated in two pull requests.
 - The pull request must pass the automated test (see test section bellow)

In your code:
 - 2 spaces indentation
 - Make sure the coding style of your code follows the style of the file.

### Run test

In order to run the tests you can run:
```
./tools/build/build
```
